export function createJog(send,notify){
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
