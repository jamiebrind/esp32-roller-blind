#include <cassert>
#include <cstdio>
#include "../ScheduledBlind/core/Scheduler.h"
int main(){
 Settings s;s.schedulesEnabled=true;s.rules={{1,true,127,EventKind::Fixed,420,0,100}};
 auto due=UkTime::toUtc({2026,6,21},420);Scheduler q;
 assert(q.dueEvents(s,due-1,due,true,true,false).size()==1);
 assert(q.dueEvents(s,due-100,due,true,true,false).empty());
 assert(q.dueEvents(s,due-1,due,false,true,false).empty());
 Scheduler unknown;assert(unknown.dueEvents(s,due-1,due,true,false,false).empty());
 Scheduler busy;assert(busy.dueEvents(s,due-1,due,true,true,true).empty());
 auto event=q.dueEvents(s,due-1,due,true,true,false)[0];q.claim(event);
 assert(q.dueEvents(s,due-1,due,true,true,false).empty());
 Scheduler skipped;assert(skipped.dueEvents(s,due-1,due,true,true,true).empty());
 assert(skipped.dueEvents(s,due-1,due,true,true,false).empty()); // Clock corrected back: skipped stays consumed.
 Scheduler restarted;restarted.restoreClaims(skipped.claims());assert(restarted.crossedEvents(s,due-1,due,true).empty());
 Settings simultaneous=s;simultaneous.rules.push_back({2,true,127,EventKind::Fixed,420,0,0});Scheduler pair;
 auto crossed=pair.crossedEvents(simultaneous,due-1,due,true);assert(crossed.size()==2);for(auto e:crossed)pair.claim(e);
 assert(pair.crossedEvents(simultaneous,due-1,due,true).empty());
 s.rules[0].localMinute=90;Scheduler fall;auto first=UkTime::toUtc({2026,10,25},90);
 auto es=fall.dueEvents(s,first-1,first,true,true,false);assert(es.size()==1);fall.claim(es[0]);
 assert(fall.dueEvents(s,first+3599,first+3600,true,true,false).empty());
 Scheduler spring;auto gap=UkTime::epoch({2026,3,29},60);assert(spring.dueEvents(s,gap-1,gap,true,true,false).size()==1);
 s.rules[0].kind=EventKind::Sunrise;assert(!spring.nextEvent(s,due));
 s.locationSet=true;s.latitude=51.5074;s.longitude=-0.1278;assert(spring.nextEvent(s,due));
 s.rules.push_back({2,true,127,EventKind::Sunrise,420,0,0});auto next=spring.nextEvent(s,due);assert(next&&next->ruleId==1);
 puts("PASS scheduler: due windows, pause/invalid-state, deduplication, DST, solar/location, stable priority");
}
