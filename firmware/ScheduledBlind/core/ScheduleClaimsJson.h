#pragma once
#include <ArduinoJson.h>
#include "Scheduler.h"
inline bool claimsNeedMigration(JsonVariantConst value){
 if(!value.is<JsonArrayConst>())return false;
 for(JsonVariantConst record:value.as<JsonArrayConst>())
  if(record["id"].is<uint32_t>()&&record["day"].is<int64_t>()&&!record["identity"].is<const char*>())return true;
 return false;
}
inline ScheduleClaims claimsFromJson(JsonVariantConst value,const Settings& settings){
 ScheduleClaims claims;if(!value.is<JsonArrayConst>())return claims;
 for(JsonVariantConst record:value.as<JsonArrayConst>()){
  if(!record["id"].is<uint32_t>()||!record["day"].is<int64_t>())continue;
  uint32_t id=record["id"];
  auto rule=std::find_if(settings.rules.begin(),settings.rules.end(),[id](const Rule& r){return r.id==id;});
  if(rule==settings.rules.end())continue;
  // Bind old 2.0.2 records to the loaded schedule: upgrading must not replay it.
  std::string identity=record["identity"].is<const char*>()?record["identity"].as<std::string>():scheduleIdentity(*rule,settings);
  claims[id]={record["day"].as<int64_t>(),identity};
 }return claims;
}
inline void claimsToJson(const ScheduleClaims& claims,const Settings& settings,JsonDocument& doc){
 doc.clear();auto array=doc.to<JsonArray>();
 for(const auto& rule:settings.rules){auto i=claims.find(rule.id);if(i==claims.end())continue;
  auto record=array.add<JsonObject>();record["id"]=rule.id;record["day"]=i->second.day;record["identity"]=i->second.identity;
 }
}
