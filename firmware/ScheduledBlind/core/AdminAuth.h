#pragma once
#include <string>
#include <functional>
#include <map>
#include <cstdint>
struct LoginResult {bool ok;std::string token,error;};
class AdminAuth {
 std::function<bool(const std::string&)> verify;std::function<std::string()> random;
 std::map<std::string,uint64_t> sessions;uint64_t elapsed=0,blockedUntil=0,window=0;uint32_t last=0;unsigned failures=0;bool begun=false;
public:
 AdminAuth(std::function<bool(const std::string&)> v,std::function<std::string()> r):verify(v),random(r){}
 void tick(uint32_t now){if(!begun){begun=true;last=now;}elapsed+=uint32_t(now-last);last=now;for(auto i=sessions.begin();i!=sessions.end();)if(elapsed>=i->second)i=sessions.erase(i);else ++i;}
 LoginResult login(const std::string& password,uint32_t now){tick(now);if(elapsed<blockedUntil)return {false,"","Too many attempts; wait one minute"};
  if(elapsed-window>=60000){window=elapsed;failures=0;}
  if(!verify(password)){if(++failures>=5){blockedUntil=elapsed+60000;failures=0;window=elapsed;}return {false,"","Password rejected or not provisioned"};}
  failures=0;if(sessions.size()>=16)return {false,"","Too many active sessions"};auto token=random();if(token.empty()||sessions.count(token))return {false,"","Session creation failed"};sessions[token]=elapsed+1800000;return {true,token,""};}
 bool authorised(const std::string& token,uint32_t now){tick(now);return !token.empty()&&sessions.count(token);}
 void logout(const std::string& token){sessions.erase(token);}
};
