#pragma once
#include <Preferences.h>
#include <ArduinoJson.h>
#include "AdminCrypto.h"
class AdminStorage {
 Preferences p;std::string salt,hash;
public:
 bool begin(){if(!p.begin("blind-admin",false))return false;JsonDocument d;if(!deserializeJson(d,p.getString("credential",""))&&d["salt"].is<const char*>()&&d["hash"].is<const char*>()){salt=d["salt"].as<std::string>();hash=d["hash"].as<std::string>();}return true;}
 bool ready()const{return salt.size()==32&&hash.size()==64;}
 bool verify(const std::string& password){return ready()&&password.size()<=63&&secureEqual(passwordHash(password,salt),hash);}
 bool provision(const std::string& password){if(password.size()<8||password.size()>63)return false;auto s=randomHex(16),h=passwordHash(password,s);if(h.empty())return false;JsonDocument d;d["salt"]=s;d["hash"]=h;String text;serializeJson(d,text);if(p.putString("credential",text)!=text.length()||p.getString("credential","")!=text)return false;salt=s;hash=h;return true;}
};
