"""PC simulation only. No serial, physical ESP or external LAN device access.
Demo password: demo-blind. All peer devices below are simulated in memory.
"""
import argparse, copy, hashlib, json, secrets, time, threading
from pathlib import Path
from http.server import ThreadingHTTPServer, SimpleHTTPRequestHandler
from urllib.parse import urlsplit, parse_qs
from email.parser import BytesParser
from email.policy import default
ROOT=Path(__file__).resolve().parents[1]/'web'
LOCK=threading.RLock()
CONFIG=dict(schemaVersion=1,sequence=1,latitude=51.5074,longitude=-.1278,locationSet=True,schedulesEnabled=False,reverseMotor=False,span=1000,encoderSign=1,maxRuntime=60000,stallMs=1000,minProgressCounts=4,ntp1='pool.ntp.org',ntp2='time.cloudflare.com',rules=[dict(id=1,enabled=True,weekdays=127,kind='sunrise',localMinute=420,offsetMinutes=0,targetPercent=100),dict(id=2,enabled=True,weekdays=127,kind='sunset',localMinute=1080,offsetMinutes=0,targetPercent=0)])
STATE=dict(preview=True,wifi=True,positionKnown=True,positionPercent=50,calibrated=True,calibrating=False,direction=0,fault='',timeValid=True,bst=True,updating=False,epoch=1,version='2.0.1',name='Study',deviceId='study',locationSet=True)
FLEET=dict(dashboard=True,name='Study',localId='study',devices=[dict(id='study',name='Study',endpoint='simulation',state='Online',positionKnown=True,positionPercent=50,direction=0,updating=False,lastContactSeconds=0,stale=False),dict(id='bedroom',name='Bedroom',endpoint='bedroom.simulated',state='Online',positionKnown=True,positionPercent=80,direction=0,updating=False,lastContactSeconds=0,stale=False),dict(id='guest',name='Guest room',endpoint='guest.simulated',state='Offline',positionKnown=False,positionPercent=0,direction=0,updating=False,lastContactSeconds=35,stale=True)],groups=[dict(id='all',name='All blinds',members=['study','bedroom','guest'])])
TOKENS={};OPS={};UPDATE=dict(state=0,received=0,error='',uploadId='',owner='',manifest=None)
class Handler(SimpleHTTPRequestHandler):
 def __init__(self,*args,**kwargs):super().__init__(*args,directory=str(ROOT),**kwargs)
 def log_message(self,*args):pass
 def reply(self,data,status=200):
  raw=json.dumps(data).encode();self.send_response(status);self.send_header('Content-Type','application/json');self.send_header('Cache-Control','no-store');self.send_header('Content-Length',str(len(raw)));self.end_headers();self.wfile.write(raw)
 def auth(self):
  token=self.headers.get('X-Blind-Auth','');return token if TOKENS.get(token,0)>time.monotonic() else ''
 def do_GET(self):
  path=urlsplit(self.path).path
  if path.startswith('/api/'):
   if not self.auth():return self.reply(dict(ok=False,error='Please log in'),401)
   with LOCK:
    if path=='/api/settings':return self.reply(CONFIG)
    if path=='/api/status':
     now=int(time.time());day=now//86400*86400;return self.reply({**STATE,'atUpper':STATE['positionKnown'] and STATE['positionPercent']>=100,'atLower':STATE['positionKnown'] and STATE['positionPercent']<=0,'rawCount':STATE['positionPercent']*10,'utc':now,'sunriseUtc':day+7*3600,'sunsetUtc':day+17*3600,'next':None})
    if path=='/api/fleet/status':
     FLEET['devices'][0].update({k:STATE[k] for k in ('positionKnown','positionPercent','direction','updating')});
     for item in FLEET['devices']:item.update(atUpper=item['positionKnown'] and item['positionPercent']>=100,atLower=item['positionKnown'] and item['positionPercent']<=0)
     return self.reply(FLEET)
    if path=='/api/fleet/key':return self.reply(dict(ok=True,deviceId='study',pairingKey='a'*64))
    if path=='/api/fleet/operation':return self.reply(dict(ok=True,members=OPS.get(parse_qs(urlsplit(self.path).query).get('id',[''])[0],[])))
    if path=='/api/firmware/info':return self.reply(dict(ok=True,name=STATE['name'],deviceId='study',hardware='esp32c3-drv8871',version=STATE['version'],maximumBytes=1966080,settingsSchema=1,automaticRollback=False,**{k:UPDATE[k] for k in ('state','received','error')},updating=STATE['updating']))
   return self.reply(dict(ok=False,error='Not found'),404)
  if path=='/':self.path='/index.html'
  return super().do_GET()
 def do_POST(self):
  global CONFIG
  path=urlsplit(self.path).path
  if self.headers.get('X-Blind-Request')!='1':return self.reply(dict(ok=False,error='Invalid origin request'),403)
  length=int(self.headers.get('Content-Length','0'))
  if length>2200000:return self.reply(dict(ok=False,error='Request too large'),413)
  raw=self.rfile.read(length)
  if path=='/api/firmware/upload':
   token=self.headers.get('X-Blind-Auth','');identity=parse_qs(urlsplit(self.path).query).get('id',[''])[0]
   if not STATE['updating'] or token!=UPDATE['owner'] or identity!=UPDATE['uploadId']:return self.reply(dict(ok=False,error='Upload session rejected'),403)
   try:
    message=BytesParser(policy=default).parsebytes(b'Content-Type: '+self.headers['Content-Type'].encode()+b'\r\nMIME-Version: 1.0\r\n\r\n'+raw)
    binary=next(part.get_payload(decode=True) for part in message.iter_parts() if part.get_param('name',header='content-disposition')=='firmware')
    m=UPDATE['manifest']
    if len(binary)!=m['length'] or hashlib.sha256(binary).hexdigest()!=m['sha256']:raise ValueError('Firmware hash or length mismatch')
    if binary[0]!=0xe9 or int.from_bytes(binary[12:14],'little')!=5 or binary[32:36]!=bytes.fromhex('3254cdab'):raise ValueError('Not an ESP32-C3 application image')
    UPDATE.update(state=4,received=len(binary));STATE.update(updating=False,version=m['version'],direction=0)
    return self.reply(dict(ok=True,simulation=True))
   except Exception as e:UPDATE.update(state=3,error=str(e));STATE['updating']=False;return self.reply(dict(ok=False,error=str(e)),400)
  try:data=json.loads(raw)
  except Exception:return self.reply(dict(ok=False,error='Invalid JSON'),400)
  if path=='/api/login':
   if data.get('password')!='demo-blind':return self.reply(dict(ok=False,error='Simulation password is demo-blind'),401)
   token=secrets.token_hex(32);TOKENS[token]=time.monotonic()+1800;return self.reply(dict(ok=True,token=token))
  token=self.auth()
  if not token:return self.reply(dict(ok=False,error='Please log in'),401)
  with LOCK:
   if path=='/api/logout':TOKENS.pop(token,None);STATE['direction']=0;return self.reply(dict(ok=True))
   if path=='/api/session':return self.reply(dict(ok=True,session=secrets.token_hex(8),epoch=STATE['epoch']))
   if path=='/api/settings':CONFIG={k:v for k,v in data.items() if k not in ('session','sequence','epoch')};return self.reply(dict(ok=True,settings=CONFIG))
   if path=='/api/firmware/prepare':
    if STATE['updating']:return self.reply(dict(ok=False,error='Update already active'),409)
    if data.get('hardware')!='esp32c3-drv8871' or data.get('settingsSchema')!=1 or not 288<=data.get('length',0)<=1966080:return self.reply(dict(ok=False,error='Incompatible manifest'),400)
    STATE.update(direction=0,updating=True,epoch=STATE['epoch']+1);UPDATE.update(state=2,received=0,error='',uploadId=secrets.token_hex(16),owner=token,manifest=data)
    return self.reply(dict(ok=True,uploadId=UPDATE['uploadId'],epoch=STATE['epoch']))
   if path=='/api/firmware/cancel':UPDATE.update(state=3,error='Cancelled');STATE['updating']=False;return self.reply(dict(ok=True))
   if path=='/api/move':
    action=data.get('action')
    if STATE['updating'] and action!='stop':return self.reply(dict(ok=False,error='Firmware update in progress'),409)
    STATE['direction']=data.get('direction',0) if action=='jog' else 0
    if action=='stop':STATE['epoch']+=1
    if action in ('raise','lower','target'):STATE['positionPercent']=100 if action=='raise' else 0 if action=='lower' else data.get('targetPercent',50)
    return self.reply(dict(ok=True,epoch=STATE['epoch']))
   if path=='/api/calibration':
    action=data.get('action');STATE['calibrating']=action=='begin' or STATE['calibrating']
    if action=='saveUpper':STATE['positionPercent']=100
    if action=='saveLower':STATE['positionPercent']=0
    if action=='finish':STATE.update(calibrated=True,positionKnown=True,calibrating=False)
    if action=='reset':STATE.update(calibrated=False,positionKnown=False,calibrating=False)
    if action=='referenceUpper':STATE.update(positionPercent=100,positionKnown=True)
    return self.reply(dict(ok=True))
   if path=='/api/fault/reset':STATE['fault']='';return self.reply(dict(ok=True))
   if path=='/api/fleet/config':
    if not 1<=len(data.get('name',''))<=32:return self.reply(dict(ok=False,error='Name must be 1–32 characters'),400)
    FLEET.update(name=data['name'],dashboard=bool(data['dashboard']));FLEET['devices'][0]['name']=STATE['name']=data['name'];return self.reply(dict(ok=True))
   if path=='/api/fleet/rotate-key':return self.reply(dict(ok=True))
   if path=='/api/fleet/pair':
    device=next((d for d in FLEET['devices'] if d['endpoint']==data.get('endpoint')),None)
    if not device or device['state']=='Offline':return self.reply(dict(ok=False,error='Simulation peer unreachable; use bedroom.simulated'),409)
    if data.get('pairingKey')!='a'*64:return self.reply(dict(ok=False,error='Simulation pairing key must be 64 a characters'),403)
    if not data.get('testOnly'):device['name']=data.get('name',device['name'])
    return self.reply(dict(ok=True,deviceId=device['id'],name=device['name']))
   if path=='/api/fleet/remove':
    identity=data.get('id')
    if identity=='study':return self.reply(dict(ok=False,error='Cannot remove this blind'),400)
    FLEET['devices']=[d for d in FLEET['devices'] if d['id']!=identity]
    for g in FLEET['groups']:g['members']=[i for i in g['members'] if i!=identity]
    FLEET['groups']=[g for g in FLEET['groups'] if g['members']];return self.reply(dict(ok=True))
   if path=='/api/fleet/group':
    identity=data.get('id');FLEET['groups']=[g for g in FLEET['groups'] if g['id']!=identity]
    if not data.get('remove'):
     members=data.get('members',[])
     if not members or any(i not in [d['id'] for d in FLEET['devices']] for i in members):return self.reply(dict(ok=False,error='Select existing members'),400)
     FLEET['groups'].append(dict(id=identity,name=data['name'],members=members))
    return self.reply(dict(ok=True))
   if path=='/api/fleet/command':
    group=next((g for g in FLEET['groups'] if g['id']==data.get('target')),None);ids=group['members'] if group else [data.get('target')];members=[]
    for d in FLEET['devices']:
     if d['id'] not in ids:continue
     state='Offline' if d['state']=='Offline' else 'Updating' if d['updating'] else 'Accepted'
     members.append(dict(id=d['id'],state=state,reason='No response (simulated)' if state=='Offline' else ''))
     if state=='Accepted':
      action=data['action'];d['direction']=0
      if action!='stop':d['positionPercent']=100 if action=='raise' else 0 if action=='lower' else data['targetPercent']
      if d['id']=='study':STATE['positionPercent']=d['positionPercent'];STATE['direction']=0
    identity=str(len(OPS)+1);OPS[identity]=members;return self.reply(dict(ok=True,operationId=identity))
  return self.reply(dict(ok=False,error='Not found'),404)
if __name__=='__main__':
 parser=argparse.ArgumentParser();parser.add_argument('--port',type=int,default=8767);args=parser.parse_args()
 print(f'Simulation preview http://127.0.0.1:{args.port}/ · password demo-blind',flush=True)
 ThreadingHTTPServer(('127.0.0.1',args.port),Handler).serve_forever()
