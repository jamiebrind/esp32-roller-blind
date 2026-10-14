#pragma once
#include <ArduinoJson.h>
#include <algorithm>
#include "Settings.h"
inline const char* kindName(EventKind k){return k==EventKind::Fixed?"fixed":k==EventKind::Sunrise?"sunrise":"sunset";}
inline void settingsToJson(const Settings& s,JsonDocument& d){
 d.clear();d["schemaVersion"]=s.schemaVersion;d["sequence"]=s.sequence;d["latitude"]=s.latitude;d["longitude"]=s.longitude;
 d["locationSet"]=s.locationSet;d["schedulesEnabled"]=s.schedulesEnabled;d["reverseMotor"]=s.reverseMotor;
 d["calibrationVersion"]=s.calibrationVersion;d["minProgressCounts"]=s.minProgressCounts;d["span"]=s.span;d["encoderSign"]=s.encoderSign;d["maxRuntime"]=s.maxRuntime;d["stallMs"]=s.stallMs;d["ntp1"]=s.ntp1;d["ntp2"]=s.ntp2;
 auto rules=d["rules"].to<JsonArray>();for(auto r:s.rules){auto o=rules.add<JsonObject>();o["id"]=r.id;o["enabled"]=r.enabled;o["weekdays"]=r.weekdays;o["kind"]=kindName(r.kind);o["localMinute"]=r.localMinute;o["offsetMinutes"]=r.offsetMinutes;o["targetPercent"]=r.targetPercent;}
}
inline std::string settingsFromJson(JsonVariantConst v,Settings& out,bool calibration){
 if(!v.is<JsonObjectConst>())return "Settings must be an object";
 const char* bools[]={"locationSet","schedulesEnabled","reverseMotor"};for(auto f:bools)if(!v[f].is<bool>())return std::string("Invalid ")+f;
 if(!v["latitude"].is<double>()||!v["longitude"].is<double>()||!v["maxRuntime"].is<uint32_t>()||!v["stallMs"].is<uint32_t>()||!v["rules"].is<JsonArrayConst>()||!v["ntp1"].is<const char*>()||!v["ntp2"].is<const char*>())return "Missing or invalid settings fields";
 Settings s=out;if(!v["minProgressCounts"].isNull()){if(!v["minProgressCounts"].is<uint32_t>())return "Invalid minimum encoder progress";s.minProgressCounts=v["minProgressCounts"];}s.locationSet=v["locationSet"];s.schedulesEnabled=v["schedulesEnabled"];s.reverseMotor=v["reverseMotor"];s.latitude=v["latitude"];s.longitude=v["longitude"];s.maxRuntime=v["maxRuntime"];s.stallMs=v["stallMs"];s.ntp1=v["ntp1"].as<std::string>();s.ntp2=v["ntp2"].as<std::string>();
 if(calibration){s.maxRuntime=std::min<uint32_t>(s.maxRuntime,60000);if(!v["span"].is<int64_t>()||!v["encoderSign"].is<int>()||!v["schemaVersion"].is<uint32_t>()||!v["sequence"].is<uint32_t>())return "Invalid stored calibration";s.calibrationVersion=v["calibrationVersion"]|0U;s.span=v["span"];s.encoderSign=v["encoderSign"];s.schemaVersion=v["schemaVersion"];s.sequence=v["sequence"];}
 s.rules.clear();if(v["rules"].size()>8)return "Maximum eight schedule rules";
 for(JsonVariantConst r:v["rules"].as<JsonArrayConst>()){
  if(!r["id"].is<uint32_t>()||!r["enabled"].is<bool>()||!r["weekdays"].is<int>()||!r["localMinute"].is<int>()||!r["offsetMinutes"].is<int>()||!r["targetPercent"].is<int>()||!r["kind"].is<const char*>())return "Invalid schedule fields";
  int days=r["weekdays"];if(days<1||days>127)return "Invalid weekdays";
  std::string k=r["kind"].as<std::string>();if(k!="fixed"&&k!="sunrise"&&k!="sunset")return "Invalid schedule type";
  s.rules.push_back({r["id"].as<uint32_t>(),r["enabled"].as<bool>(),uint8_t(days),k=="fixed"?EventKind::Fixed:k=="sunrise"?EventKind::Sunrise:EventKind::Sunset,r["localMinute"].as<int>(),r["offsetMinutes"].as<int>(),r["targetPercent"].as<int>()});
 }
 auto error=SettingsValidator::validate(s);if(!error.empty())return error;out=std::move(s);return {};
}
