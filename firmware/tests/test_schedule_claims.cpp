#include <cassert>
#include <cstdio>
#include "../ScheduledBlind/core/ScheduleClaimsJson.h"
#include "../ScheduledBlind/core/SettingsJson.h"
int main(){
 Settings s;s.schedulesEnabled=true;s.rules={{2,true,127,EventKind::Fixed,1350,0,0}};
 auto time=UkTime::toUtc({2026,10,14},1350);JsonDocument old;
 auto r=old.to<JsonArray>().add<JsonObject>();r["id"]=2;r["day"]=UkTime::days({2026,10,14});
 auto migrated=claimsFromJson(old.as<JsonVariantConst>(),s);Scheduler q;q.restoreClaims(migrated);
 assert(claimsNeedMigration(old.as<JsonVariantConst>()));
 JsonDocument migration;claimsToJson(migrated,s,migration);
 assert(!claimsNeedMigration(migration.as<JsonVariantConst>()));
 assert(q.crossedEvents(s,time-1,time,true).empty());
 Settings edited=s;edited.rules[0].localMinute=1355;auto later=time+300;
 assert(q.crossedEvents(edited,later-1,later,true).size()==1);
 JsonDocument stored;claimsToJson(migrated,edited,stored);
 Scheduler reboot;q.restoreClaims(claimsFromJson(stored.as<JsonVariantConst>(),edited));
 auto event=q.crossedEvents(edited,later-1,later,true);assert(event.size()==1);q.claim(event[0]);
 claimsToJson(q.claims(),edited,stored);reboot.restoreClaims(claimsFromJson(stored.as<JsonVariantConst>(),edited));
 assert(reboot.crossedEvents(edited,later-1,later,true).empty());
 Settings solar;solar.schedulesEnabled=true;solar.locationSet=true;solar.latitude=51.507412345678;solar.longitude=-0.127812345678;
 solar.rules={{3,true,127,EventKind::Sunset,420,0,0}};
 Settings canonical;std::string payload;assert(canonicalSettingsPayload(solar,canonical,payload));
 auto identity=scheduleIdentity(canonical.rules[0],canonical);JsonDocument settingsDoc;assert(!deserializeJson(settingsDoc,payload));
 Settings loaded;assert(settingsFromJson(settingsDoc.as<JsonVariantConst>(),loaded,true).empty());
 assert(identity==scheduleIdentity(loaded.rules[0],loaded));
 Settings removed=edited;removed.rules.clear();claimsToJson(q.claims(),removed,stored);assert(stored.size()==0);
 puts("PASS schedule claims: legacy no replay, edited identity retained across save/restart, new claim dedup");
}
