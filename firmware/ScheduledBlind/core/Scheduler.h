#pragma once
#include "Settings.h"
#include "Solar.h"
#include <algorithm>
#include <map>
#include <cstring>
// Store the complete schedule identity, avoiding hash collisions and tying claims
// to the settings that actually produced an occurrence. Ignore inactive fields.
inline std::string scheduleIdentity(const Rule& r,const Settings& s){
 std::string value=std::to_string(int(r.kind))+":"+std::to_string(r.enabled)+":"+std::to_string(r.weekdays)+":"+std::to_string(r.targetPercent);
 if(r.kind==EventKind::Fixed)return value+":"+std::to_string(r.localMinute);
 uint64_t lat=0,lon=0;static_assert(sizeof(double)==sizeof(uint64_t));
 std::memcpy(&lat,&s.latitude,sizeof(lat));std::memcpy(&lon,&s.longitude,sizeof(lon));
 return value+":"+std::to_string(r.offsetMinutes)+":"+std::to_string(s.locationSet)+":"+std::to_string(lat)+":"+std::to_string(lon);
}
struct ScheduleClaim{int64_t day;std::string identity;};
using ScheduleClaims=std::map<uint32_t,ScheduleClaim>;
struct Event{uint32_t ruleId;int64_t localDay,utc;int targetPercent;std::string identity;};
class Scheduler{
 ScheduleClaims claimed;
 std::vector<Event> events(const Settings& s,int64_t now){
  std::vector<Event> out;if(!s.schedulesEnabled)return out;
  int64_t day=UkTime::days(UkTime::fromUtc(now).date);
  for(int delta=-1;delta<=8;++delta){Date d=UkTime::dateFromDays(day+delta);
   for(auto r:s.rules){if(!r.enabled||!(r.weekdays&(1<<UkTime::weekday(d))))continue;
    std::optional<int64_t> t;
    if(r.kind==EventKind::Fixed)t=UkTime::toUtc(d,r.localMinute);
    else if(s.locationSet){auto sun=Solar::events(d,s.latitude,s.longitude);t=r.kind==EventKind::Sunrise?sun.sunriseUtc:sun.sunsetUtc;if(t)*t+=r.offsetMinutes*60;}
    auto identity=scheduleIdentity(r,s);auto claim=claimed.find(r.id);
    if(t && (claim==claimed.end()||claim->second.identity!=identity||claim->second.day<day+delta))out.push_back({r.id,day+delta,*t,r.targetPercent,identity});
   }
  }
  std::sort(out.begin(),out.end(),[](Event a,Event b){return a.utc==b.utc?a.ruleId<b.ruleId:a.utc<b.utc;});return out;
 }
public:
 std::vector<Event> crossedEvents(const Settings& s,int64_t previous,int64_t now,bool clockValid){
  std::vector<Event> out;if(!clockValid||now<=previous||now-previous>30)return out;
  for(auto e:events(s,now))if(e.utc>previous&&e.utc<=now)out.push_back(e);return out;
 }
 std::vector<Event> dueEvents(const Settings& s,int64_t previous,int64_t now,bool clockValid,bool positionKnown,bool busy){
  std::vector<Event> out;if(!clockValid||now<=previous||now-previous>30)return out;
  if(!positionKnown||busy){for(auto e:crossedEvents(s,previous,now,true))claim(e);return out;}
  for(auto e:events(s,now))if(e.utc>previous&&e.utc<=now)out.push_back(e);
  std::sort(out.begin(),out.end(),[](Event a,Event b){return a.ruleId<b.ruleId;});return out;
 }
 std::optional<Event> nextEvent(const Settings& s,int64_t now){for(auto e:events(s,now))if(e.utc>now)return e;return {};}
 void claim(Event e){auto it=claimed.find(e.ruleId);if(it==claimed.end()||it->second.identity!=e.identity||e.localDay>it->second.day)claimed[e.ruleId]={e.localDay,e.identity};}
 const ScheduleClaims& claims()const{return claimed;}
 void restoreClaims(ScheduleClaims values){claimed=std::move(values);}
};
