#pragma once
#include "FleetRegistry.h"
#include <optional>
#include <cstdint>
#include <deque>
#include <map>
#include <set>
struct PeerResult {std::string state,reason;};struct PeerWork {uint32_t id,operation;Device device;std::string action;int percent;bool poll=false;};struct MemberResult {std::string id,state,reason;};
class FleetDispatch {
 std::deque<PeerWork> queue;std::map<uint32_t,PeerWork> active;std::map<uint32_t,std::vector<MemberResult>> operations;uint32_t nextId=0,nextOperation=0;std::set<uint32_t> cancelled;
 void member(PeerWork w,PeerResult result){auto i=operations.find(w.operation);if(i!=operations.end())for(auto& r:i->second)if(r.id==w.device.id){r.state=result.state;r.reason=result.reason;}}
public:
 uint32_t submit(std::vector<Device> snapshot,std::string action,int percent,uint32_t now){
  if(snapshot.empty()||snapshot.size()>8||percent<0||percent>100||(action!="raise"&&action!="lower"&&action!="stop"&&action!="target"))return 0;
  if(action=="stop"){for(auto i=queue.begin();i!=queue.end();)if(!i->poll&&std::any_of(snapshot.begin(),snapshot.end(),[&](auto d){return d.id==i->device.id;})){member(*i,{"Rejected","Superseded by Stop"});i=queue.erase(i);}else ++i;}
  if(action=="stop")for(auto& entry:active){auto& w=entry.second;if(!w.poll&&w.action!="stop"&&std::any_of(snapshot.begin(),snapshot.end(),[&](auto d){return d.id==w.device.id;})){cancelled.insert(w.id);member(w,{"Rejected","Superseded by Stop"});}}
  while(operations.size()>=8){auto i=std::find_if(operations.begin(),operations.end(),[](auto& o){return std::none_of(o.second.begin(),o.second.end(),[](auto m){return m.state=="Pending";});});if(i==operations.end()){if(action!="stop")return 0;break;}operations.erase(i);}
  if(action=="stop")while(queue.size()+snapshot.size()>64){
   auto i=std::find_if(queue.begin(),queue.end(),[](const PeerWork& w){return w.poll;});
   if(i==queue.end())i=std::find_if(queue.begin(),queue.end(),[](const PeerWork& w){return w.action!="stop";});
   if(i==queue.end())break;member(*i,{"Rejected","Superseded to admit Stop"});queue.erase(i);
  }
  if(queue.size()+snapshot.size()>64)return 0;
  uint32_t op=++nextOperation;auto& members=operations[op];for(auto d:snapshot){members.push_back({d.id,"Pending",""});PeerWork w{++nextId,op,d,action,percent,false};if(action=="stop")queue.push_front(w);else queue.push_back(w);}return op;
 }
 bool current(uint32_t id)const{return active.count(id)&&!cancelled.count(id);}
 void cancelPending(){for(auto& entry:active)if(!entry.second.poll&&entry.second.action!="stop"){cancelled.insert(entry.first);member(entry.second,{"Rejected","Dashboard maintenance"});}for(auto i=queue.begin();i!=queue.end();)if(i->action!="stop"){if(!i->poll)member(*i,{"Rejected","Dashboard maintenance"});i=queue.erase(i);}else ++i;}
 bool poll(Device d){if(queue.size()>=64||std::any_of(queue.begin(),queue.end(),[&](auto w){return w.device.id==d.id&&w.poll;}))return false;queue.push_back({++nextId,0,d,"",0,true});return true;}
 std::optional<PeerWork> nextWork(uint32_t now){if(active.size()>=2)return {};for(auto i=queue.begin();i!=queue.end();++i){if(std::any_of(active.begin(),active.end(),[&](auto w){return w.second.device.id==i->device.id;}))continue;auto work=*i;queue.erase(i);active[work.id]=work;return work;}return {};}
 void complete(uint32_t id,PeerResult result,uint32_t now){auto i=active.find(id);if(i==active.end())return;if(cancelled.erase(id))result={"Rejected","Superseded by Stop or maintenance"};member(i->second,result);active.erase(i);}
 std::vector<MemberResult> results(uint32_t op)const{auto i=operations.find(op);return i==operations.end()?std::vector<MemberResult>{}:i->second;}
 static bool stale(uint32_t now,uint32_t last){return uint32_t(now-last)>10000;}
};
