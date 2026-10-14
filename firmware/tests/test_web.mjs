import test from 'node:test';import assert from 'node:assert/strict';
import {createJog,validateRule,createSettingsStore} from '../web/control.mjs';
test('release emits Stop and never restarts after a late response',async()=>{
 const sent=[];let resolve;const transport=msg=>{sent.push(msg.action);return new Promise(r=>{resolve=r;});};
 const jog=createJog(transport,()=>{});jog.start(1);jog.stop();assert.deepEqual(sent,['jog','stop']);resolve({ok:true});await new Promise(r=>setTimeout(r,10));assert.deepEqual(sent,['jog','stop']);
});
test('rejected save retains the last active settings',async()=>{
 const store=createSettingsStore({schedulesEnabled:false},async()=>({ok:false,error:'save failed'}));
 await assert.rejects(()=>store.save({schedulesEnabled:true}),/save failed/);
 assert.equal(store.current().schedulesEnabled,false);
});
test('failed connection ends manual jogging',async()=>{
 let count=0;const jog=createJog(async()=>{++count;throw Error('offline');},()=>{});jog.start(1);await new Promise(r=>setTimeout(r,10));assert.equal(jog.active(),false);assert.equal(count,2);
});
test('rule requires valid time, weekdays and range',()=>{
 assert.equal(validateRule({id:1,weekdays:127,kind:'fixed',localMinute:420,offsetMinutes:0,targetPercent:100}),null);
 assert.ok(validateRule({id:1,weekdays:0,kind:'fixed',localMinute:420,offsetMinutes:0,targetPercent:100}));
 assert.ok(validateRule({id:1,weekdays:127,kind:'fixed',localMinute:1500,offsetMinutes:0,targetPercent:101}));
});

test('only the initial press can create a jog lease',async()=>{
 const sent=[];const jog=createJog(async msg=>{sent.push(msg);return {ok:true};},()=>{});
 jog.start(1);await new Promise(r=>setTimeout(r,180));jog.stop();
 assert.equal(sent[0].begin,true);assert.equal(sent[1].begin,false);assert.equal(sent.at(-1).action,'stop');
});

import {validateFirmware} from '../web/firmware.mjs';
test('firmware rejects wrong target, overlength, mismatched file and invalid manifest',()=>{
 const m={hardware:'esp32c3-drv8871',version:'2.0.0',settingsSchema:1,length:300,sha256:'a'.repeat(64)};
 assert.equal(validateFirmware({name:'firmware.bin',size:300},m),null);
 assert.ok(validateFirmware({name:'firmware.bin',size:300},{...m,hardware:'esp32'}));
 assert.ok(validateFirmware({name:'firmware.bin',size:300},{...m,length:301}));
 assert.ok(validateFirmware({name:'firmware.bin',size:1966081},{...m,length:1966081}));
 assert.ok(validateFirmware({name:'firmware.bin',size:300},{...m,settingsSchema:2}));
});

import {fleetResultText} from '../web/fleet.mjs';
test('group results include every member and never claim completed travel',()=>{
 const text=fleetResultText([{id:'a',state:'Accepted',reason:''},{id:'b',state:'Offline',reason:'No response'}]);
 assert.match(text,/a: Accepted/);assert.match(text,/b: Offline/);assert.doesNotMatch(text,/completed/i);
});

import {motionAvailability} from '../web/control.mjs';
test('normal controls reject reached endpoints and remain blocked if position is unknown',()=>{
 const state={positionKnown:true,calibrated:true,atUpper:true,atLower:false};assert.equal(motionAvailability(state).raise,false);assert.equal(motionAvailability(state).lower,true);
 assert.equal(motionAvailability({...state,atUpper:false,atLower:true}).lower,false);
 assert.equal(motionAvailability({...state,positionKnown:false}).lower,false);
});

import {firmwareUpload} from '../web/firmware.mjs';
test('wireless upload waits for applied Stop and binds the issued upload session',async()=>{
 const events=[];const file={name:'app.bin',size:300};const manifest={hardware:'esp32c3-drv8871',version:'2.0.1',settingsSchema:1,length:300,sha256:'a'.repeat(64)};
 const result=await firmwareUpload(file,manifest,{prepare:async()=>{events.push('prepare');return {ok:true,uploadId:'owner-session'};},ready:async()=>events.push('stopped'),upload:async(f,id,progress)=>{assert.equal(f,file);assert.equal(id,'owner-session');events.push('upload');progress(100);return {ok:true};},cancel:async()=>events.push('cancel')});assert.equal(result.ok,true);assert.deepEqual(events,['prepare','stopped','upload']);
});
test('wireless transfer failure cancels maintenance without claiming success',async()=>{
 const events=[];const manifest={hardware:'esp32c3-drv8871',version:'2.0.1',settingsSchema:1,length:300,sha256:'a'.repeat(64)};
 await assert.rejects(()=>firmwareUpload({name:'app.bin',size:300},manifest,{prepare:async()=>({ok:true,uploadId:'owner'}),ready:async()=>{},upload:async()=>({ok:false,error:'hash mismatch'}),cancel:async()=>events.push('cancel')}),/hash mismatch/);assert.deepEqual(events,['cancel']);
});
