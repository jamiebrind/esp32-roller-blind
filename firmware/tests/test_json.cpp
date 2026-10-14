#include <cassert>
#include <cstdio>
#include "../ScheduledBlind/core/SettingsJson.h"
int main(){
 Settings s;s.locationSet=true;s.latitude=51;s.longitude=-.1;s.span=500;s.rules={{1,true,127,EventKind::Sunset,420,15,0}};
 JsonDocument d;settingsToJson(s,d);Settings restored;assert(settingsFromJson(d.as<JsonVariantConst>(),restored,true).empty());
 assert(restored.span==500 && restored.rules.size()==1 && restored.rules[0].offsetMinutes==15);
 d["rules"][0]["targetPercent"]=-1;assert(!settingsFromJson(d.as<JsonVariantConst>(),restored,true).empty());
 settingsToJson(s,d);d["latitude"]="invalid";assert(!settingsFromJson(d.as<JsonVariantConst>(),restored,true).empty());
 settingsToJson(s,d);d["span"]=1000;restored=s;assert(settingsFromJson(d.as<JsonVariantConst>(),restored,false).empty());assert(restored.span==500);
 settingsToJson(s,d);d["maxRuntime"]=120000;assert(settingsFromJson(d.as<JsonVariantConst>(),restored,true).empty());assert(restored.maxRuntime==60000);
 assert(!settingsFromJson(d.as<JsonVariantConst>(),restored,false).empty());
 puts("PASS JSON: settings round-trip, invalid typed values, calibration protected from settings edits");
}
