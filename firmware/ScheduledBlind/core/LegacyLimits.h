#pragma once
#include "SettingsJson.h"
inline std::optional<Settings> legacyLimits(const std::string& record){
 JsonDocument outer,inner;if(deserializeJson(outer,record)||!outer["payload"].is<const char*>()||!outer["checksum"].is<uint32_t>())return {};
 auto payload=outer["payload"].as<std::string>();if(checksum(payload)!=outer["checksum"].as<uint32_t>()||deserializeJson(inner,payload))return {};
 if(!inner["version"].is<uint32_t>()||inner["version"].as<uint32_t>()!=1||!inner["generation"].is<uint32_t>()||!inner["span"].is<int64_t>()||!inner["sign"].is<int>())return {};
 Settings s;s.span=inner["span"].as<int64_t>();s.encoderSign=inner["sign"].as<int>();s.calibrationVersion=inner["generation"].as<uint32_t>();
 if(s.span<=0||!SettingsValidator::validate(s).empty())return {};return s;
}
