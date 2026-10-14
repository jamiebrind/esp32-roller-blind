export function fleetResultText(members){return members.map(m=>`${m.name||m.id}: ${m.state}${m.reason?' — '+m.reason:''}`).join('\n');}
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
