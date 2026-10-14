"""Local in-memory simulation transport test; never contacts a real blind."""
import hashlib, importlib.util, json, threading, unittest, urllib.request
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
spec=importlib.util.spec_from_file_location('preview',ROOT/'tools/preview_server.py');preview=importlib.util.module_from_spec(spec);spec.loader.exec_module(preview)
class Upload(unittest.TestCase):
 def test_local_multipart_matches_manifest(self):
  server=preview.ThreadingHTTPServer(('127.0.0.1',0),preview.Handler);thread=threading.Thread(target=server.serve_forever,daemon=True);thread.start()
  base='http://127.0.0.1:'+str(server.server_port);token=''
  def post(path,data,content_type='application/json'):
   headers={'Content-Type':content_type,'X-Blind-Request':'1','X-Blind-Auth':token}
   with urllib.request.urlopen(urllib.request.Request(base+path,data=data,headers=headers),timeout=5) as r:return json.load(r)
  try:
   token=post('/api/login',json.dumps({'password':'demo-blind'}).encode())['token']
   binary=(ROOT/'build/ScheduledBlind.ino.bin').read_bytes();manifest=json.loads((ROOT/'build/manifest.json').read_text(encoding='utf-8'))
   self.assertEqual(hashlib.sha256(binary).hexdigest(),manifest['sha256'])
   prepared=post('/api/firmware/prepare',json.dumps(manifest).encode());identity=prepared['uploadId']
   boundary='blind-test-boundary';raw=(f'--{boundary}\r\nContent-Disposition: form-data; name="firmware"; filename="app.bin"\r\nContent-Type: application/octet-stream\r\n\r\n'.encode()+binary+f'\r\n--{boundary}--\r\n'.encode())
   result=post('/api/firmware/upload?id='+identity,raw,'multipart/form-data; boundary='+boundary)
   self.assertTrue(result['ok']);self.assertTrue(result['simulation']);self.assertEqual(preview.UPDATE['received'],len(binary));self.assertFalse(preview.STATE['updating'])
  finally:server.shutdown();server.server_close();thread.join(timeout=2)
if __name__=='__main__':unittest.main()
