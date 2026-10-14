#include <cassert>
#include <cstdio>
#include "../ScheduledBlind/core/Scheduler.h"
int main(){
 const Date date{2026,10,14};
 Settings before;before.schedulesEnabled=true;
 before.rules={{1,true,127,EventKind::Fixed,480,0,100},{2,true,127,EventKind::Fixed,1350,0,0}};
 auto original=UkTime::toUtc(date,1350),now=UkTime::toUtc(date,1354),editedTime=UkTime::toUtc(date,1355);
 Scheduler q;auto originalEvents=q.crossedEvents(before,original-1,original,true);
 assert(originalEvents.size()==1);q.claim(originalEvents[0]);
 auto untouched=q.nextEvent(before,now);assert(untouched&&untouched->localDay>UkTime::days(date));
 Settings edited=before;edited.rules[1].localMinute=1355;
 auto next=q.nextEvent(edited,now);
 assert(next&&next->ruleId==2&&next->utc==editedTime); // Failed on 2.0.2: old claim suppresses the edited rule.
 assert(q.crossedEvents(edited,now,now+1,true).empty()); // Saving must not cause an immediate movement.
 auto due=q.crossedEvents(edited,editedTime-1,editedTime,true);assert(due.size()==1);q.claim(due[0]);
 assert(q.crossedEvents(edited,editedTime-1,editedTime,true).empty());
 Scheduler rebooted;rebooted.restoreClaims(q.claims());assert(rebooted.crossedEvents(edited,editedTime-1,editedTime,true).empty());
 edited.rules[1].localMinute=1320;
 assert(rebooted.crossedEvents(edited,now,now+1,true).empty());
 auto past=rebooted.nextEvent(edited,now);assert(past&&past->localDay>UkTime::days(date));
 edited=before;edited.rules[1].targetPercent=50;
 assert(q.crossedEvents(edited,original-1,original,true).size()==1);
 Settings solar;solar.schedulesEnabled=true;solar.locationSet=true;solar.latitude=51.5074;solar.longitude=-0.1278;
 solar.rules={{3,true,127,EventKind::Sunset,420,0,0}};Scheduler sun;
 auto sunset=sun.nextEvent(solar,UkTime::toUtc(date,0));assert(sunset);sun.claim(*sunset);
 solar.rules[0].offsetMinutes=10;
 auto later=sun.nextEvent(solar,sunset->utc+60);assert(later&&later->utc==sunset->utc+600);
 solar.rules[0].offsetMinutes=0;solar.latitude=52.0;
 auto moved=sun.nextEvent(solar,UkTime::toUtc(date,0));assert(moved&&moved->localDay==sunset->localDay);
 puts("PASS schedule edits: later today, unchanged dedup, restart, no catch-up, target and solar edits");
}
