#pragma once
#include <Preferences.h>
#include "core/SettingsJson.h"
#include "core/LegacyLimits.h"
#include "core/Motion.h"
#include "core/Scheduler.h"
#include "core/ScheduleClaimsJson.h"
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
  Settings next=s;next.sequence++;Settings candidate;std::string payload;if(!canonicalSettingsPayload(next,candidate,payload))return false;
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
 bool migrateClaims(const Settings& settings){
  JsonDocument old;if(deserializeJson(old,p.getString("claims","[]")))return true;
  if(!claimsNeedMigration(old.as<JsonVariantConst>()))return true;
  auto values=claimsFromJson(old.as<JsonVariantConst>(),settings);JsonDocument upgraded;claimsToJson(values,settings,upgraded);
  String text;serializeJson(upgraded,text);
  // Migration must reach NVS before any schedule edit can be accepted.
  return p.putString("claims",text)==text.length()&&p.getString("claims","")==text;
 }
 ScheduleClaims loadClaims(const Settings& settings){
  JsonDocument d;if(deserializeJson(d,p.getString("claims","[]")))return {};
  return claimsFromJson(d.as<JsonVariantConst>(),settings);
 }
 bool claimEvent(const Event& e,const Scheduler& scheduler,const Settings& settings){
  auto values=scheduler.claims();values[e.ruleId]={e.localDay,e.identity};JsonDocument d;claimsToJson(values,settings,d);
  String text;serializeJson(d,text);return p.putString("claims",text)==text.length();
 }
};
