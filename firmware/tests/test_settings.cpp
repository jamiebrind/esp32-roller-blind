#include <cassert>
#include <cstdio>
#include "../ScheduledBlind/core/Settings.h"
int main(){
 Settings s;assert(SettingsValidator::validate(s).empty());
 s.locationSet=true;s.latitude=91;assert(!SettingsValidator::validate(s).empty());s.latitude=51;
 s.rules.push_back({1,true,127,EventKind::Fixed,420,0,100});assert(SettingsValidator::validate(s).empty());
 s.rules.push_back(s.rules[0]);assert(!SettingsValidator::validate(s).empty());s.rules.pop_back();
 s.rules[0].targetPercent=101;assert(!SettingsValidator::validate(s).empty());s.rules[0].targetPercent=100;
 s.rules[0].weekdays=0;assert(!SettingsValidator::validate(s).empty());s.rules[0].weekdays=127;
 s.span=-1;assert(!SettingsValidator::validate(s).empty());s.span=0;
 StorageRecord old{1,2,"valid",true},broken{1,3,"partial",false};assert(selectRecord(old,broken)->payload=="valid");
 StorageRecord future{2,4,"future",true};assert(selectRecord(old,future)->payload=="valid");
 puts("PASS settings: validation, duplicate IDs, target/weekday limits, interrupted/schema recovery");
}
