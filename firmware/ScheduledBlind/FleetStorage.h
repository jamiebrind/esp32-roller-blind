#pragma once
#include <Preferences.h>
#include <ArduinoJson.h>
#include "core/FleetRegistry.h"
#include "core/Settings.h"
#include "AdminCrypto.h"
inline void fleetJson(const FleetConfig& c,JsonDocument& d,bool secrets){d["dashboard"]=c.dashboard;d["name"]=c.name;auto devices=d["devices"].to<JsonArray>();for(auto x:c.devices){auto r=devices.add<JsonObject>();r["id"]=x.id;r["name"]=x.name;r["endpoint"]=x.endpoint;if(secrets)r["pairingKey"]=x.pairingKey;}auto groups=d["groups"].to<JsonArray>();for(auto g:c.groups){auto r=groups.add<JsonObject>();r["id"]=g.id;r["name"]=g.name;auto members=r["members"].to<JsonArray>();for(auto id:g.members)members.add(id);}}
class FleetStorage {
 Preferences p;uint32_t sequence=0;std::string key;
 bool parse(String raw,Device local,FleetConfig& config,uint32_t& seq){JsonDocument env;if(deserializeJson(env,raw)||!env["payload"].is<const char*>()||!env["checksum"].is<uint32_t>()||!env["sequence"].is<uint32_t>())return false;auto payload=env["payload"].as<std::string>();if(checksum(payload)!=env["checksum"].as<uint32_t>())return false;JsonDocument d;if(deserializeJson(d,payload)||!d["dashboard"].is<bool>()||!d["name"].is<const char*>()||!d["devices"].is<JsonArray>()||!d["groups"].is<JsonArray>())return false;
  FleetRegistry r(local);r.config.dashboard=d["dashboard"];local.name=d["name"].as<std::string>();if(!r.edit(local).empty())return false;bool own=false;
  for(JsonVariant v:d["devices"].as<JsonArray>()){if(!v["id"].is<const char*>()||!v["name"].is<const char*>()||!v["endpoint"].is<const char*>())return false;Device x{v["id"].as<std::string>(),v["name"].as<std::string>(),v["endpoint"].as<std::string>(),v["pairingKey"]|""};if(x.id==local.id){if(own)return false;own=true;}else if(!r.add(x).empty())return false;}
  if(!own)return false;for(JsonVariant v:d["groups"].as<JsonArray>()){if(!v["id"].is<const char*>()||!v["name"].is<const char*>()||!v["members"].is<JsonArray>())return false;Group g{v["id"].as<std::string>(),v["name"].as<std::string>(),{}};for(JsonVariant id:v["members"].as<JsonArray>()){if(!id.is<const char*>())return false;g.members.push_back(id.as<std::string>());}if(!r.saveGroup(g).empty())return false;}config=r.config;seq=env["sequence"];return true;
 }
public:
 bool begin(){if(!p.begin("blind-fleet",false))return false;key=p.getString("pairing","").c_str();if(key.size()!=64){key=randomHex();if(p.putString("pairing",key.c_str())!=key.size())return false;}return true;}
 std::string pairingKey()const{return key;}
 bool rotate(){auto candidate=randomHex();if(p.putString("pairing",candidate.c_str())!=candidate.size())return false;key=candidate;return true;}
 FleetConfig load(Device local){FleetConfig a,b;uint32_t sa=0,sb=0;bool av=parse(p.getString("fleet0",""),local,a,sa),bv=parse(p.getString("fleet1",""),local,b,sb);if(!av&&!bv)return FleetRegistry(local).config;if(bv&&(!av||int32_t(sb-sa)>0)){sequence=sb;return b;}sequence=sa;return a;}
 bool save(const FleetConfig& config){JsonDocument inner;fleetJson(config,inner,true);std::string payload;serializeJson(inner,payload);JsonDocument envelope;envelope["version"]=1;envelope["sequence"]=sequence+1;envelope["checksum"]=checksum(payload);envelope["payload"]=payload;String text;serializeJson(envelope,text);const char* slot=(sequence+1)%2?"fleet1":"fleet0";if(p.putString(slot,text)!=text.length()||p.getString(slot,"")!=text)return false;++sequence;return true;}
};
