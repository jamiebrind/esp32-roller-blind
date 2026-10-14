#pragma once
const char INDEX_HTML[] PROGMEM = R"WEBASSET(<!doctype html><html lang="en"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1"><title>Blind controller</title><link rel="stylesheet" href="/app.css"></head>

<body><main><header><div><small>LOCAL BLIND CONTROLLER</small><h1>Your blind</h1></div><span id="connection" class="pill">Connecting…</span></header>

<div id="preview" hidden class="notice">Preview only · simulated blind · nothing controls the physical device</div>

<div id="message" role="status" aria-live="polite"></div>

<section id="login-panel" class="panel"><h2>Log in</h2><p>Use the administrator password set during USB setup. This local page uses your trusted home Wi-Fi.</p><form id="login-form"><label>Password<input id="password" type="password" autocomplete="current-password" required maxlength="63"></label><button class="primary">Log in</button></form></section><div id="authenticated" hidden>

<nav aria-label="Sections"><button data-tab="control" class="selected">Control</button><button data-tab="calibration">Calibration</button><button data-tab="schedules">Schedules</button><button data-tab="settings">Settings</button><button id="fleet-tab" data-tab="blinds" hidden>Blinds</button><button data-tab="firmware">Firmware</button><button id="logout">Log out</button></nav><button id="stop-global" class="danger">■ Stop this blind</button>

<section id="control" class="panel"><div class="position"><div class="blind-icon">▤</div><div><h2 id="position">—</h2><p id="motion">Waiting for device</p></div></div>

<div class="actions"><button id="raise" class="primary">↑ Raise</button><button id="stop" class="danger">■ Stop</button><button id="lower" class="primary">↓ Lower</button></div>

<label for="target">Target position <span id="target-value">50%</span></label><input id="target" type="range" min="0" max="100" value="50"><button id="go">Move to position</button>

<div class="facts"><div><small>NEXT SCHEDULE</small><p id="next">—</p></div><div><small>DEVICE CLOCK</small><p id="clock">—</p></div></div>

<p class="hint">0% is fully lowered. 100% is fully raised. Travel limits need calibration before automatic movement.</p></section>

<section id="calibration" class="panel" hidden><h2>Set your travel limits</h2><p>Automation is paused during calibration. Hold a jog control, release at the desired position, then save that limit.</p>

<button id="begin-calibration" class="primary">Begin calibration</button><div class="actions"><button id="jog-up">Hold to jog ↑</button><button id="jog-down">Hold to jog ↓</button></div>

<ol><li>Move to fully raised, then <button id="save-upper">Save upper limit</button></li><li>Move to fully lowered, then <button id="save-lower">Save lower limit</button></li><li><button id="finish-calibration" class="primary">Finish and save calibration</button></li></ol>

<details><summary>Recover position or clear a fault</summary><p>Only confirm the upper reference when the blind is physically at your saved upper limit.</p><button id="reference-upper">Blind is at upper limit</button><button id="reset-fault">Acknowledge fault</button><button id="reset-calibration">Reset calibration</button></details>

<p>Live encoder count: <strong id="encoder-count">—</strong></p><p class="hint">Each continuous run is capped at 60 seconds. A feedback fault during calibration requires starting again. The encoder tracks motor rotation. If the chain slips or is moved by hand, correct the reference. Software limits are not physical end switches.</p></section>

<section id="schedules" class="panel" hidden><div class="section-title"><h2>Schedules</h2><button id="add-rule">+ Add rule</button></div><label class="check"><input id="enable-schedules" type="checkbox"> Enable automatic schedules</label>

<div class="facts"><p>Sunrise <strong id="sunrise">—</strong></p><p>Sunset <strong id="sunset">—</strong></p></div><div id="rules"></div><button id="save-schedules" class="primary">Save schedules</button><p class="hint">Rules run on the device. Keep the clock synchronised and calibrate travel before enabling automation.</p></section>

<section id="settings" class="panel" hidden><h2>Device setup</h2><label>Blind name<input id="device-name" maxlength="32"></label><label class="check"><input id="dashboard-role" type="checkbox"> Show the multi-blind dashboard on this device</label><button id="save-device">Save device setup</button><details><summary>Pairing this blind</summary><p>Log in to another dashboard and enter this blind's hostname/IP and pairing key. Treat the key like a password.</p><button id="show-pair-key">Show pairing key</button><output id="pair-key" class="key-output"></output><button id="rotate-pair-key">Replace pairing key</button></details><h2>Location &amp; time</h2><p>Sunrise and sunset are calculated on the device using your coordinates.</p>

<label class="check"><input id="location-set" type="checkbox"> Use this location for sunrise/sunset</label><div class="two"><label>Latitude<input id="latitude" type="number" step="any" min="-90" max="90"></label><label>Longitude<input id="longitude" type="number" step="any" min="-180" max="180"></label></div>

<label>Timezone<input value="Europe/London · automatic GMT/BST" disabled></label><div class="two"><label>NTP server<input id="ntp1" maxlength="100"></label><label>Fallback NTP server<input id="ntp2" maxlength="100"></label></div>

<label class="check"><input id="reverse-motor" type="checkbox"> Reverse motor direction</label>

<details><summary>Motor safeguards</summary><label>Maximum continuous run (seconds)<input id="max-runtime" type="number" min="1" max="60"></label><label>No-movement timeout after startup (seconds)<input id="stall-ms" type="number" min="0.1" max="5" step="0.1"></label><label>Minimum encoder counts in 2 seconds<input id="min-progress" type="number" min="1" max="100000"></label><p class="hint">These must be checked on your real assembly. Encoder feedback does not measure pull force.</p></details>

<button id="save-settings" class="primary">Save settings</button></section>

<section id="blinds" class="panel" hidden><h2>Your blinds</h2><p>Each blind keeps its own limits and schedules. Group commands start approximately together.</p><div id="fleet-devices" class="fleet-grid"></div><h2>Groups</h2><div id="fleet-groups" class="fleet-grid"></div><pre id="group-results" role="status"></pre>
<details><summary>Add or edit a blind</summary><label>Saved blind<select id="edit-device"><option value="">Add a new blind</option></select></label><label>Name<input id="pair-name" maxlength="32"></label><label>Hostname or IP<input id="pair-endpoint" placeholder="blind-abc123.local"></label><label>Pairing key<input id="pair-secret" type="password" autocomplete="off" maxlength="64"></label><div class="actions"><button id="test-pair">Test connection</button><button id="save-pair">Save blind</button><button id="remove-peer">Remove blind</button></div></details>
<details><summary>Add or edit a group</summary><label>Saved group<select id="edit-group"><option value="">Create a group</option></select></label><label>Group name<input id="group-name" maxlength="32"></label><div id="group-members"></div><div class="actions"><button id="save-group">Save group</button><button id="remove-group">Remove group</button></div></details></section>
<section id="firmware" class="panel" hidden><h2>Firmware updates</h2><p id="firmware-info">Checking version…</p><p class="hint">Choose the application .bin and its matching manifest. The motor and schedules pause while updating. Each device is updated individually.</p><label>Application firmware<input id="firmware-file" type="file" accept=".bin"></label><label>Release manifest<input id="manifest-file" type="file" accept=".json"></label><div class="actions"><button id="upload-firmware" class="primary">Install firmware</button><button id="cancel-firmware">Cancel update</button></div><progress id="upload-progress" max="100" value="0"></progress><p id="upload-message" role="status"></p></section>

</div><footer>Your settings stay on this device</footer></main><script type="module" src="/app.js"></script></body></html>

)WEBASSET";
const char APP_CSS[] PROGMEM = R"WEBASSET(:root{font-family:system-ui,-apple-system,sans-serif;color:#18372e;background:#edf3ed;font-size:16px}*{box-sizing:border-box}body{margin:0}main{max-width:820px;margin:auto;padding:32px 20px}header{display:flex;align-items:center;justify-content:space-between;gap:12px;margin-bottom:22px}small{font-size:11px;letter-spacing:.12em;color:#64776e}h1{font-size:34px;letter-spacing:-.04em;margin:6px 0}h2{font-size:22px;margin:0 0 16px}p{line-height:1.6;color:#54685e}.pill{font-size:13px;border-radius:25px;background:#dce9dc;padding:9px 13px}nav{display:flex;gap:6px;padding:5px;background:#e0e9e0;border-radius:14px;margin-bottom:20px}nav button{flex:1;border:0;background:transparent;padding:12px 8px}nav .selected{background:white;box-shadow:0 2px 8px #264d2110}.panel{background:#fff;border:1px solid #dbe4da;border-radius:20px;padding:28px;box-shadow:0 7px 24px #263e2410}button{font:600 15px system-ui;background:#eaf0e9;border:1px solid #d3dfd3;color:#214b3d;border-radius:11px;padding:13px 17px;cursor:pointer;touch-action:manipulation}button:hover{filter:brightness(.97)}button:disabled{opacity:.45;cursor:default}.primary{background:#275d47;color:white;border-color:#275d47}.danger{background:#f4e2de;border-color:#ead1c9;color:#8c3428}.actions{display:flex;gap:10px;margin:22px 0}.actions button{flex:1;padding:19px 10px}.position{display:flex;gap:20px;align-items:center}.position h2{font-size:44px;letter-spacing:-.05em;margin:0}.position p{margin:5px 0}.blind-icon{font-size:55px;color:#426c54;background:#eef4ea;width:95px;text-align:center;border-radius:14px;padding:12px}label{display:block;font-size:14px;font-weight:600;margin:18px 0 7px}input:not([type=checkbox]):not([type=range]),select{display:block;width:100%;font:16px system-ui;padding:11px;border:1px solid #cedbcd;border-radius:9px;margin-top:7px;background:#fff;color:#244337}input[type=range]{width:100%;accent-color:#275d47;margin:15px 0 19px}input:disabled{background:#f2f4f0!important}input[type=checkbox]{accent-color:#275d47}.check{display:flex;align-items:center;gap:9px}.facts,.two{display:grid;grid-template-columns:1fr 1fr;gap:18px}.facts{border-top:1px solid #e8ede6;margin-top:24px;padding-top:20px}.facts p{margin:8px 0}.hint{font-size:13px;color:#6c7f73}ol{padding-left:24px}li{margin:18px 0;line-height:1.6}li button{margin-left:8px}details{margin:22px 0}summary{cursor:pointer;font-weight:600}footer{text-align:center;color:#7d8e80;font-size:12px;margin-top:25px}.rule{border:1px solid #dbe4d9;border-radius:13px;padding:16px;margin:16px 0}.section-title,.rule-head{display:flex;justify-content:space-between;align-items:center;gap:12px}.days{display:flex;flex-wrap:wrap;gap:8px;margin-top:16px}.days label{margin:0;font-size:12px}.notice{padding:12px;border-radius:10px;background:#fff0ca;color:#74551e;margin-bottom:16px}#message:not(:empty){background:#fff0dc;color:#765219;border-radius:10px;padding:12px;margin-bottom:16px}#target-value{float:right}[hidden]{display:none!important}#jog-up,#jog-down{touch-action:none;user-select:none}@media(max-width:540px){main{padding:20px 12px}.panel{padding:20px}nav button{font-size:12px;padding:12px 5px}h1{font-size:28px}.two{grid-template-columns:1fr;gap:0}.facts{gap:10px}.position h2{font-size:36px}.actions{gap:6px}.actions button{font-size:14px}.pill{max-width:130px;text-align:center}}

nav{flex-wrap:wrap}nav button{min-width:80px}progress{width:100%;margin:16px 0}.fleet-card{border:1px solid #dbe4d9;border-radius:13px;padding:16px;margin:12px 0}.fleet-grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(250px,1fr));gap:12px}

.device-link{display:block;margin-top:18px;color:#275d47}.key-output{display:block;overflow-wrap:anywhere;margin:16px 0}pre{white-space:pre-wrap;overflow-wrap:anywhere;font:14px system-ui;line-height:1.7}
)WEBASSET";
const char APP_JS[] PROGMEM = R"WEBASSET(import {renderFleet,fleetResultText} from '/fleet.mjs';
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
)WEBASSET";
const char CONTROL_JS[] PROGMEM = R"WEBASSET(export function createJog(send,notify){
 let held=0,generation=0,timer=null;
 function stop(message='Stopped'){
  held=0;++generation;clearTimeout(timer);notify(message);
  Promise.resolve(send({action:'stop'})).catch(()=>notify('Connection lost; jog timeout will stop movement'));
 }
 async function pulse(version,begin=false){
  if(!held||version!==generation)return;
  try{const result=await send({action:'jog',direction:held,begin});if(!result.ok)throw Error(result.error||'Jog rejected');
   if(held&&version===generation)timer=setTimeout(()=>pulse(version),150);
  }catch(e){if(version===generation)stop(e.message);}
 }
 return {start(direction){if(held)return;held=direction;notify(direction===1?'Jogging up':'Jogging down');pulse(++generation,true);},stop,active:()=>!!held};
}
export function validateRule(r){
 if(!Number.isInteger(r.id)||r.id<1||!Number.isInteger(r.weekdays)||r.weekdays<1||r.weekdays>127)return 'Select at least one weekday';
 if(!['fixed','sunrise','sunset'].includes(r.kind))return 'Choose a schedule type';
 if(!Number.isInteger(r.localMinute)||r.localMinute<0||r.localMinute>1439)return 'Enter a valid time';
 if(!Number.isInteger(r.offsetMinutes)||Math.abs(r.offsetMinutes)>720)return 'Offset must be within 12 hours';
 if(!Number.isInteger(r.targetPercent)||r.targetPercent<0||r.targetPercent>100)return 'Position must be between 0% and 100%';
 return null;
}
export function createSettingsStore(initial,send){
 let active=structuredClone(initial);
 return {current:()=>structuredClone(active),async save(candidate){const result=await send(candidate);if(!result.ok)throw Error(result.error||'Settings could not be saved');active=structuredClone(result.settings||candidate);return structuredClone(active);}};
}

export function motionAvailability(s){const blocked=s.updating||!s.calibrated||!s.positionKnown||!!s.fault||s.calibrating;return {raise:!blocked&&!s.atUpper,lower:!blocked&&!s.atLower,target:!blocked};}
)WEBASSET";
const char FIRMWARE_JS[] PROGMEM = R"WEBASSET(export function validateFirmware(file,m){
 if(!file||!m||m.hardware!=='esp32c3-drv8871'||m.settingsSchema!==1)return 'Firmware target or settings version is incompatible';
 if(!Number.isInteger(m.length)||m.length<288||m.length>1966080||file.size!==m.length)return 'Firmware file size does not match its manifest';
 if(typeof m.version!=='string'||!m.version||m.version.length>32||!(/^[a-f0-9]{64}$/).test(m.sha256))return 'Invalid firmware manifest';
 if(!file.name.endsWith('.bin'))return 'Choose the application .bin file';return null;
}
export async function firmwareUpload(file,manifest,transport,onProgress=()=>{}){
 const error=validateFirmware(file,manifest);if(error)throw Error(error);
 const prepared=await transport.prepare(manifest);if(!prepared.ok)throw Error(prepared.error||'Update preparation rejected');
 try{await transport.ready();const result=await transport.upload(file,prepared.uploadId,onProgress);if(!result.ok)throw Error(result.error||'Firmware verification failed');return result;}
 catch(e){try{await transport.cancel();}catch{}throw e;}
}
)WEBASSET";
const char FLEET_JS[] PROGMEM = R"WEBASSET(export function fleetResultText(members){return members.map(m=>`${m.name||m.id}: ${m.state}${m.reason?' — '+m.reason:''}`).join('\n');}
export function renderFleet(snapshot,handlers){
 const devices=document.getElementById('fleet-devices'),groups=document.getElementById('fleet-groups');
 const values=new Map([...document.querySelectorAll('[data-fleet-target]')].map(e=>[e.dataset.fleetTarget,e.value]));
 const focused=document.activeElement?.dataset?.fleetTarget;
 devices.replaceChildren();groups.replaceChildren();
 function card(parent,item,isGroup){const article=document.createElement('article');article.className='fleet-card';const title=document.createElement('h3');title.textContent=item.name;article.append(title);
  const status=document.createElement('p');status.textContent=isGroup?`${item.members.length} blinds`:item.stale?'Status stale · '+(item.state||'Offline'):item.updating?'Updating':item.fault?'Fault: '+item.fault:item.direction?'Moving · '+item.positionPercent+'%':`${item.positionKnown?item.positionPercent+'% open':'Position unknown'} · ${item.state||'Offline'}`;article.append(status);
  if(!isGroup){const contact=document.createElement('p');contact.className='hint';contact.textContent=item.lastContactSeconds>=0?`Last contact ${item.lastContactSeconds}s ago`:'No successful contact yet';article.append(contact);}
  const actions=document.createElement('div');actions.className='actions';for(const [action,label] of [['raise','Raise'],['stop','Stop'],['lower','Lower']]){const button=document.createElement('button');button.textContent=label;button.setAttribute('aria-label',`${label} ${item.name}`);if(action==='stop')button.className='danger';if(!isGroup&&action!=='stop')button.disabled=!!item.updating||!!item.fault||!item.positionKnown||(!item.stale&&(action==='raise'?item.atUpper:item.atLower));button.onclick=()=>handlers.command(item.id,action,action==='raise'?100:0);actions.append(button);}article.append(actions);
  const label=document.createElement('label');label.textContent='Open percentage';const input=document.createElement('input');input.type='number';input.min=0;input.max=100;input.dataset.fleetTarget=(isGroup?'group:':'device:')+item.id;input.value=values.get(input.dataset.fleetTarget)||'50';label.append(input);article.append(label);const go=document.createElement('button');go.textContent='Move to percentage';go.onclick=()=>handlers.command(item.id,'target',Number(input.value));article.append(go);
  if(!isGroup){const link=document.createElement('a');link.textContent='Open device / firmware';link.href=item.id===snapshot.localId?'#firmware':`http://${item.endpoint}/#firmware`;link.target='_blank';link.rel='noopener';link.className='device-link';article.append(link);}
  parent.append(article);if(focused===input.dataset.fleetTarget)input.focus();
 }
 for(const d of snapshot.devices)card(devices,d,false);for(const g of snapshot.groups)card(groups,g,true);
}
)WEBASSET";
