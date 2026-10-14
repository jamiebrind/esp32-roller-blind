#pragma once
#include <Preferences.h>
#include "core/SettingsJson.h"
#include "core/LegacyLimits.h"
#include "core/Motion.h"
#include "core/Scheduler.h"
class DeviceStorage{
 Preferences p;
 StorageRecord record(const char* key){
  String raw=p.getString(key,"");JsonDocument d;StorageRecord r{0,0,"",false};
  if(deserializeJson(d,raw)||!d["payload"].is<const char*>())return r;
  r.version=d["version"]|0U;r.sequence=d["sequence"]|0U;r.payload=d["payload"].as<std::string>();
  r.checksumValid=d["checksum"].is<uint32_t>()&&checksum(r.payload)==d["checksum"].as<uint32_t>();
  JsonDocument inner;Settings s;if(deserializeJson(inner,r.payload)||!settingsFromJson(inner.as<JsonVariantConst>(),s,true).empty())r.checksumValid=false;
  return r;
 }
public:
 bool begin(){return p.begin("blind-scheduled",false);}
 bool migrateManualLimits(){
  if(p.isKey("settings0")||p.isKey("settings1"))return true;
  Preferences old;if(!old.begin("blind-limits",true))return true;
  String text=old.getString("state","");old.end();auto legacy=legacyLimits(text.c_str());if(!legacy)return true;
  Settings candidate=*legacy;
  return saveSettings(candidate)&&savePosition({0,false,false,candidate.calibrationVersion});
 }
 Settings loadSettings(){
  Settings s;auto selected=selectRecord(record("settings0"),record("settings1"));
  if(selected){JsonDocument d;if(!deserializeJson(d,selected->payload))settingsFromJson(d.as<JsonVariantConst>(),s,true);}return s;
 }
 bool saveSettings(Settings& s){
  Settings candidate=s;candidate.sequence++;JsonDocument inner;settingsToJson(candidate,inner);std::string payload;serializeJson(inner,payload);
  JsonDocument d;d["version"]=1;d["sequence"]=candidate.sequence;d["checksum"]=checksum(payload);d["payload"]=payload;String text;serializeJson(d,text);
  const char* key=candidate.sequence%2?"settings1":"settings0";
  if(p.putString(key,text)!=text.length())return false;
  auto read=record(key);if(!read.checksumValid||read.sequence!=candidate.sequence)return false;s=candidate;return true;
 }
 PositionRecord loadPosition(){
  String text=p.getString("position","");JsonDocument d;if(deserializeJson(d,text)||!d["count"].is<int64_t>()||!d["valid"].is<bool>()||!d["moving"].is<bool>())return {0,false,true};
  return {d["count"].as<int64_t>(),d["valid"].as<bool>(),d["moving"].as<bool>(),d["calibrationVersion"]|0U};
 }
 bool savePosition(PositionRecord pos){JsonDocument d;d["calibrationVersion"]=pos.calibrationVersion;d["count"]=pos.count;d["valid"]=pos.valid;d["moving"]=pos.moving;String text;serializeJson(d,text);return p.putString("position",text)==text.length();}
 bool markMoving(PositionRecord pos){pos.moving=true;return savePosition(pos);}
 std::map<uint32_t,int64_t> loadClaims(){
  JsonDocument d;std::map<uint32_t,int64_t> values;if(deserializeJson(d,p.getString("claims","[]")))return values;
  for(JsonVariant r:d.as<JsonArray>())if(r["id"].is<uint32_t>()&&r["day"].is<int64_t>())values[r["id"].as<uint32_t>()]=r["day"].as<int64_t>();return values;
 }
 bool claimEvent(const Event& e,const Scheduler& scheduler,const Settings& settings){
  auto values=scheduler.claims();values[e.ruleId]=e.localDay;JsonDocument d;auto a=d.to<JsonArray>();
  for(const auto& rule:settings.rules){auto i=values.find(rule.id);if(i!=values.end()){auto r=a.add<JsonObject>();r["id"]=i->first;r["day"]=i->second;}}
  String text;serializeJson(d,text);return p.putString("claims",text)==text.length();
 }
};
