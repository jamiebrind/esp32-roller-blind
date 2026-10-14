#pragma once
#include "Settings.h"
#include "Solar.h"
#include <algorithm>
#include <map>
struct Event{uint32_t ruleId;int64_t localDay,utc;int targetPercent;};
class Scheduler{
 std::map<uint32_t,int64_t> claimed;
 std::vector<Event> events(const Settings& s,int64_t now){
  std::vector<Event> out;if(!s.schedulesEnabled)return out;
  int64_t day=UkTime::days(UkTime::fromUtc(now).date);
  for(int delta=-1;delta<=8;++delta){Date d=UkTime::dateFromDays(day+delta);
   for(auto r:s.rules){if(!r.enabled||!(r.weekdays&(1<<UkTime::weekday(d))))continue;
    std::optional<int64_t> t;
    if(r.kind==EventKind::Fixed)t=UkTime::toUtc(d,r.localMinute);
    else if(s.locationSet){auto sun=Solar::events(d,s.latitude,s.longitude);t=r.kind==EventKind::Sunrise?sun.sunriseUtc:sun.sunsetUtc;if(t)*t+=r.offsetMinutes*60;}
    if(t && (!claimed.count(r.id)||claimed[r.id]<day+delta))out.push_back({r.id,day+delta,*t,r.targetPercent});
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
 void claim(Event e){auto it=claimed.find(e.ruleId);if(it==claimed.end()||e.localDay>it->second)claimed[e.ruleId]=e.localDay;}
 const std::map<uint32_t,int64_t>& claims()const{return claimed;}
 void restoreClaims(std::map<uint32_t,int64_t> values){claimed=std::move(values);}
};
