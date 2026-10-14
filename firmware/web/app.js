import {renderFleet,fleetResultText} from '/fleet.mjs';
import {firmwareUpload} from '/firmware.mjs';

import {createJog,validateRule,createSettingsStore,motionAvailability} from '/control.mjs';

const $=id=>document.getElementById(id);

let adminToken='',pollTimer=null;

let session='',epoch=1,sequence=0,store=null,draft=null,lastStatus=null,fleetSnapshot=null,lastFleetPoll=0;

function message(text){$('message').textContent=text;}

async function api(path,body){

 const options={cache:'no-store',headers:{'X-Blind-Auth':adminToken}};

 if(body!==undefined){options.method='POST';options.headers={...options.headers,'Content-Type':'application/json','X-Blind-Request':'1'};options.body=JSON.stringify({...body,session,epoch,sequence:++sequence});options.keepalive=true;}

 const response=await fetch(path,options);const data=await response.json();if(Number.isInteger(data.epoch))epoch=data.epoch;if(response.status===401){showLogin();}if(!response.ok||data.ok===false)throw Error(data.error||'Request failed');return data;

}

async function sendMove(data){try{await api('/api/move',data);return {ok:true};}catch(e){return {ok:false,error:e.message};}}

const jog=createJog(sendMove,message);

function dateTime(utc){return utc?new Date(utc*1000).toLocaleString('en-GB',{timeZone:'Europe/London',dateStyle:'medium',timeStyle:'short'}):'—';}

function timeOnly(utc){return utc?new Date(utc*1000).toLocaleTimeString('en-GB',{timeZone:'Europe/London',hour:'2-digit',minute:'2-digit'}):lastStatus?.locationSet?'No event today':'Set location';}

function renderStatus(s){

 lastStatus=s;if(Number.isInteger(s.epoch))epoch=s.epoch;$('preview').hidden=!s.preview;$('connection').textContent=s.preview?'Simulation':s.wifi?'Wi-Fi connected':'Wi-Fi offline';

 $('position').textContent=s.positionKnown?`${s.positionPercent}% open`:'Position unknown';

 $('motion').textContent=s.fault?'Fault: '+s.fault:s.calibrating?'Calibration in progress':s.direction===1?'Raising':s.direction===-1?'Lowering':s.calibrated?'Stopped':'Calibration required';

 $('clock').textContent=s.timeValid?dateTime(s.utc)+(s.bst?' · BST':' · GMT'):'Waiting for NTP';

 $('sunrise').textContent=timeOnly(s.sunriseUtc);$('sunset').textContent=timeOnly(s.sunsetUtc);

 $('next').textContent=s.next?`${s.next.targetPercent===100?'Raise':s.next.targetPercent===0?'Lower':s.next.targetPercent+'%'} · ${dateTime(s.next.utc)}`:'No scheduled action';

 const automatic=s.updating||!s.calibrated||!s.positionKnown||!!s.fault||s.calibrating;

 const available=motionAvailability(s);$('raise').disabled=!available.raise;$('lower').disabled=!available.lower;$('go').disabled=!available.target;
 $('jog-up').disabled=!!s.updating||!!s.fault||!!s.atUpper;$('jog-down').disabled=!!s.updating||!!s.fault||!!s.atLower;
 $('encoder-count').textContent=String(s.rawCount??s.count);
 for(const id of ['save-upper','save-lower','finish-calibration'])$(id).disabled=!!s.updating||!!s.direction||!!s.fault||!s.calibrating;
 $('reference-upper').disabled=!!s.updating||!!s.direction||!s.calibrated||s.calibrating;

}

async function poll(){if(!adminToken)return;try{renderStatus(await api('/api/status'));if(Date.now()-lastFleetPoll>3000){lastFleetPoll=Date.now();await refreshFleet();}}catch(e){$('connection').textContent='Connection lost';message('Device not reachable; check Wi-Fi');}finally{if(adminToken)pollTimer=setTimeout(poll,1000);}}

function renderSettings(s){

 $('enable-schedules').checked=s.schedulesEnabled;$('location-set').checked=s.locationSet;$('latitude').value=s.latitude;$('longitude').value=s.longitude;

 $('ntp1').value=s.ntp1;$('ntp2').value=s.ntp2;$('reverse-motor').checked=s.reverseMotor;$('max-runtime').value=s.maxRuntime/1000;$('stall-ms').value=s.stallMs/1000;$('min-progress').value=s.minProgressCounts??4;renderRules();

}

function renderRules(){

 $('rules').replaceChildren();

 for(const rule of draft.rules){

  const node=document.createElement('article');node.className='rule';

  node.innerHTML=`<div class="rule-head"><label class="check"><input class="enabled" type="checkbox"> Enabled</label><button class="remove">Remove</button></div><div class="two"><label>Trigger<select class="kind"><option value="fixed">Custom time</option><option value="sunrise">Sunrise</option><option value="sunset">Sunset</option></select></label><label>Position<select class="position"><option value="100">Raise fully</option><option value="0">Lower fully</option><option value="custom">Custom position</option></select></label></div><div class="two"><label class="time-label">Time<input class="time" type="time"></label><label class="offset-label">Offset in minutes<input class="offset" type="number" min="-720" max="720"><small>Negative = before, positive = after</small></label><label class="percent-label">Open percentage<input class="percent" type="number" min="0" max="100"></label></div><div class="days"></div>`;

  node.querySelector('.enabled').checked=rule.enabled;node.querySelector('.kind').value=rule.kind;

  node.querySelector('.position').value=[0,100].includes(rule.targetPercent)?String(rule.targetPercent):'custom';node.querySelector('.percent').value=rule.targetPercent;

  node.querySelector('.time').value=`${String(Math.floor(rule.localMinute/60)).padStart(2,'0')}:${String(rule.localMinute%60).padStart(2,'0')}`;node.querySelector('.offset').value=rule.offsetMinutes;

  ['Sun','Mon','Tue','Wed','Thu','Fri','Sat'].forEach((day,i)=>{const label=document.createElement('label');const input=document.createElement('input');input.type='checkbox';input.checked=!!(rule.weekdays&(1<<i));input.dataset.day=i;label.append(input,document.createTextNode(day));node.querySelector('.days').append(label);});

  function update(){const [h,m]=node.querySelector('.time').value.split(':').map(Number);rule.enabled=node.querySelector('.enabled').checked;rule.kind=node.querySelector('.kind').value;rule.localMinute=Number.isFinite(h+m)?h*60+m:NaN;rule.offsetMinutes=Number(node.querySelector('.offset').value);const pos=node.querySelector('.position').value;rule.targetPercent=pos==='custom'?Number(node.querySelector('.percent').value):Number(pos);rule.weekdays=[...node.querySelectorAll('[data-day]')].reduce((mask,e)=>mask|(e.checked?1<<Number(e.dataset.day):0),0);node.querySelector('.time-label').hidden=rule.kind!=='fixed';node.querySelector('.offset-label').hidden=rule.kind==='fixed';node.querySelector('.percent-label').hidden=pos!=='custom';}

  node.addEventListener('change',update);node.querySelector('.remove').onclick=()=>{draft.rules=draft.rules.filter(r=>r!==rule);renderRules();};$('rules').append(node);update();

 }

}

function candidate(){const s=structuredClone(draft);s.schedulesEnabled=$('enable-schedules').checked;s.locationSet=$('location-set').checked;s.latitude=Number($('latitude').value);s.longitude=Number($('longitude').value);s.ntp1=$('ntp1').value.trim();s.ntp2=$('ntp2').value.trim();s.reverseMotor=$('reverse-motor').checked;s.maxRuntime=Number($('max-runtime').value)*1000;s.stallMs=Number($('stall-ms').value)*1000;s.minProgressCounts=Number($('min-progress').value);return s;}

async function save(){try{const s=candidate();for(const r of s.rules){const error=validateRule(r);if(error)throw Error(error);}draft=await store.save(s);renderSettings(draft);message('Saved on device');}catch(e){message(e.message);}}

async function command(path,data){try{await api(path,data);message('Done');}catch(e){message(e.message);}}

for(const button of document.querySelectorAll('nav button[data-tab]'))button.onclick=()=>{if(jog.active())jog.stop();for(const section of document.querySelectorAll('section.panel'))section.hidden=section.id!==button.dataset.tab;for(const b of document.querySelectorAll('nav button'))b.classList.toggle('selected',b===button);};

$('raise').onclick=()=>command('/api/move',{action:'raise'});$('lower').onclick=()=>command('/api/move',{action:'lower'});$('stop').onclick=()=>jog.stop();$('stop-global').onclick=()=>jog.stop();$('go').onclick=()=>command('/api/move',{action:'target',targetPercent:Number($('target').value)});$('target').oninput=()=>{$('target-value').textContent=$('target').value+'%';};

for(const [id,direction] of [['jog-up',1],['jog-down',-1]]){

 const button=$(id);button.addEventListener('pointerdown',e=>{if(e.button!==0)return;e.preventDefault();button.setPointerCapture(e.pointerId);jog.start(direction);});

 for(const event of ['pointerup','pointercancel','lostpointercapture'])button.addEventListener(event,()=>{if(jog.active())jog.stop();});

 button.addEventListener('keydown',e=>{if([' ','Enter'].includes(e.key)&&!e.repeat){e.preventDefault();jog.start(direction);}});button.addEventListener('keyup',e=>{if([' ','Enter'].includes(e.key)){e.preventDefault();jog.stop();}});

}

for(const [id,action] of [['begin-calibration','begin'],['save-upper','saveUpper'],['save-lower','saveLower'],['finish-calibration','finish'],['reference-upper','referenceUpper'],['reset-calibration','reset']])$(id).onclick=()=>{if(['begin','reset','referenceUpper'].includes(action)&&!confirm(action==='referenceUpper'?'Confirm the blind is exactly at the saved upper position?':'Replace the current travel calibration?'))return;command('/api/calibration',{action});};

$('reset-fault').onclick=()=>command('/api/fault/reset',{});

$('add-rule').onclick=()=>{if(draft.rules.length>=8){message('Maximum eight rules');return;}draft.rules.push({id:Math.max(0,...draft.rules.map(r=>r.id))+1,enabled:true,weekdays:127,kind:'fixed',localMinute:420,offsetMinutes:0,targetPercent:100});renderRules();};

$('save-schedules').onclick=save;$('save-settings').onclick=save;

for(const event of ['blur','pagehide'])window.addEventListener(event,()=>{if(jog.active())jog.stop();});document.addEventListener('visibilitychange',()=>{if(document.hidden&&jog.active())jog.stop();});

async function init(){try{const auth=await api('/api/session',{});session=auth.session;sequence=0;const s=await api('/api/settings');store=createSettingsStore(s,async value=>{try{return await api('/api/settings',value);}catch(e){return {ok:false,error:e.message};}});draft=store.current();renderSettings(draft);await refreshFleet();if(location.hash==='#firmware')document.querySelector('[data-tab=firmware]').click();poll();}catch(e){message(e.message);if(adminToken)setTimeout(init,3000);}}

function showLogin(){adminToken='';clearTimeout(pollTimer);$('authenticated').hidden=true;$('login-panel').hidden=false;session='';}

$('login-form').onsubmit=async e=>{e.preventDefault();try{const result=await api('/api/login',{password:$('password').value});adminToken=result.token;$('password').value='';$('login-panel').hidden=true;$('authenticated').hidden=false;await init();await updateInfo();}catch(e){message(e.message);}};

$('logout').onclick=async()=>{if(jog.active())jog.stop();try{await api('/api/logout',{});}finally{showLogin();}};

async function updateInfo(){const s=await api('/api/firmware/info');$('firmware-info').textContent=`${s.name||'This blind'} · ${s.version} · ${Math.floor(s.maximumBytes/1024)} KB maximum. Recovery from faulty firmware may require USB.`;return s;}

$('upload-firmware').onclick=async()=>{let button=$('upload-firmware');button.disabled=true;try{

 const file=$('firmware-file').files[0],mf=$('manifest-file').files[0];if(!mf)throw Error('Choose the release manifest');const manifest=JSON.parse(await mf.text());

 await firmwareUpload(file,manifest,{

 prepare:m=>api('/api/firmware/prepare',m),

 ready:async()=>{for(let i=0;i<50;i++){const info=await updateInfo();if(info.state===2)return;if(!info.updating)throw Error(info.error||'Update preparation failed');await new Promise(r=>setTimeout(r,100));}throw Error('Motor has not stopped for update');},

 cancel:()=>api('/api/firmware/cancel',{}),

 upload:(f,id,progress)=>new Promise((resolve,reject)=>{const xhr=new XMLHttpRequest();xhr.open('POST',`/api/firmware/upload?id=${encodeURIComponent(id)}`);xhr.setRequestHeader('X-Blind-Auth',adminToken);xhr.setRequestHeader('X-Blind-Request','1');xhr.timeout=120000;xhr.upload.onprogress=e=>{if(e.lengthComputable)progress(Math.round(e.loaded/e.total*100));};xhr.onload=()=>{try{const data=JSON.parse(xhr.responseText);if(xhr.status>=400)throw Error(data.error||'Upload rejected');resolve(data);}catch(e){reject(e);}};xhr.onerror=()=>reject(Error('Upload connection lost'));xhr.ontimeout=()=>reject(Error('Upload timed out'));const data=new FormData();data.append('firmware',f);xhr.send(data);})

 },value=>{$('upload-progress').value=value;$('upload-message').textContent=`Uploading ${value}%…`;});$('upload-message').textContent='Firmware verified. Device is restarting; reconnect and log in.';

 }catch(e){$('upload-message').textContent=e.message;}finally{button.disabled=false;}};

$('cancel-firmware').onclick=()=>command('/api/firmware/cancel',{});

showLogin();


async function refreshFleet(){
 const snapshot=await api('/api/fleet/status');fleetSnapshot=snapshot;$('fleet-tab').hidden=!snapshot.dashboard;$('dashboard-role').checked=snapshot.dashboard;
 if(document.activeElement!==$('device-name'))$('device-name').value=snapshot.name;
 if(snapshot.dashboard)renderFleet(snapshot,{command:fleetCommand});
 const deviceSelect=$('edit-device'),groupSelect=$('edit-group');const selected=deviceSelect.value,selectedGroup=groupSelect.value;
 deviceSelect.replaceChildren(new Option('Add a new blind',''));for(const d of snapshot.devices)if(d.id!==snapshot.localId)deviceSelect.add(new Option(d.name,d.id));deviceSelect.value=selected;
 groupSelect.replaceChildren(new Option('Create a group',''));for(const g of snapshot.groups)groupSelect.add(new Option(g.name,g.id));groupSelect.value=selectedGroup;
 if(!$('group-members').children.length)renderMembers([]);
}
function renderMembers(selected){$('group-members').replaceChildren();for(const d of fleetSnapshot.devices){const label=document.createElement('label');label.className='check';const input=document.createElement('input');input.type='checkbox';input.value=d.id;input.checked=selected.includes(d.id);label.append(input,document.createTextNode(d.name));$('group-members').append(label);}}
async function fleetCommand(target,action,percent){try{const result=await api('/api/fleet/command',{target,action,targetPercent:percent});for(let i=0;i<120&&adminToken;i++){const status=await api('/api/fleet/operation?id='+result.operationId);const members=status.members.map(m=>({...m,name:fleetSnapshot.devices.find(d=>d.id===m.id)?.name||m.id}));$('group-results').textContent=fleetResultText(members);if(!members.some(m=>m.state==='Pending'))break;await new Promise(r=>setTimeout(r,500));}await refreshFleet();}catch(e){message(e.message);}}
$('save-device').onclick=async()=>{try{await api('/api/fleet/config',{name:$('device-name').value.trim(),dashboard:$('dashboard-role').checked});await refreshFleet();message('Device setup saved');}catch(e){message(e.message);}};
$('show-pair-key').onclick=async()=>{try{$('pair-key').textContent=(await api('/api/fleet/key')).pairingKey;}catch(e){message(e.message);}};
$('rotate-pair-key').onclick=async()=>{try{await api('/api/fleet/rotate-key',{});$('pair-key').textContent='';message('Pairing key replaced. Re-pair this blind on other dashboards.');}catch(e){message(e.message);}};
$('edit-device').onchange=()=>{const d=fleetSnapshot.devices.find(d=>d.id===$('edit-device').value);$('pair-name').value=d?.name||'';$('pair-endpoint').value=d?.endpoint||'';$('pair-secret').value='';};
async function pair(testOnly){try{const result=await api('/api/fleet/pair',{id:$('edit-device').value,name:$('pair-name').value.trim(),endpoint:$('pair-endpoint').value.trim(),pairingKey:$('pair-secret').value.trim(),testOnly});if(testOnly)message('Connected to '+(result.name||result.deviceId));else{$('pair-secret').value='';await refreshFleet();message('Blind saved');}}catch(e){message(e.message);}}
$('test-pair').onclick=()=>pair(true);$('save-pair').onclick=()=>pair(false);
$('remove-peer').onclick=async()=>{try{await api('/api/fleet/remove',{id:$('edit-device').value});await refreshFleet();message('Blind removed');}catch(e){message(e.message);}};
$('edit-group').onchange=()=>{const g=fleetSnapshot.groups.find(g=>g.id===$('edit-group').value);$('group-name').value=g?.name||'';renderMembers(g?.members||[]);};
async function saveGroup(remove=false){try{await api('/api/fleet/group',{id:$('edit-group').value||'g-'+Date.now().toString(36)+'-'+Math.random().toString(36).slice(2,6),name:$('group-name').value.trim(),members:[...$('group-members').querySelectorAll('input:checked')].map(e=>e.value),remove});await refreshFleet();message(remove?'Group removed':'Group saved');}catch(e){message(e.message);}}
$('save-group').onclick=()=>saveGroup();$('remove-group').onclick=()=>saveGroup(true);
