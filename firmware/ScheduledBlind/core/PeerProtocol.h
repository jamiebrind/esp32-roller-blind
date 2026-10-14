#pragma once
#include <string>
#include <cstdint>
#include <map>
#include <deque>
struct PeerCommand {std::string commandId,nonce;uint32_t sequence,epoch;std::string action;int targetPercent;};struct PeerDecision {bool accepted,duplicate;std::string reason;uint32_t epoch;};
class PeerProtocol {
 struct Session{uint32_t sequence=0;uint64_t expiry;};struct Cached{std::string id,payload;PeerDecision reply;};
 std::map<std::string,Session> sessions;std::deque<Cached> cache;uint32_t generation=1,last=0;uint64_t elapsed=0;bool begun=false;std::string boot;
 void tick(uint32_t now){if(!begun){last=now;begun=true;}elapsed+=uint32_t(now-last);last=now;for(auto i=sessions.begin();i!=sessions.end();)if(elapsed>=i->second.expiry)i=sessions.erase(i);else ++i;}
public:
 explicit PeerProtocol(std::string nonce):boot(nonce){}
 bool open(std::string token,uint32_t now){tick(now);if(token.empty()||sessions.size()>=8||sessions.count(token))return false;sessions[token]={0,elapsed+1800000};return true;}
 PeerDecision accept(PeerCommand c,bool authenticated,uint32_t now){tick(now);auto reject=[&](std::string s){return PeerDecision{false,false,s,generation};};if(!authenticated)return reject("Pairing key rejected");auto i=sessions.find(c.nonce);if(i==sessions.end())return reject("Peer session expired or wrong boot");
  std::string payload=c.nonce+":"+std::to_string(c.sequence)+":"+std::to_string(c.epoch)+":"+c.action+":"+std::to_string(c.targetPercent);
  for(auto& x:cache)if(x.id==c.commandId){if(x.payload!=payload)return reject("Command ID reused with changed content");auto r=x.reply;r.duplicate=true;return r;}
  if(c.commandId.empty()||c.commandId.size()>64||!c.sequence||c.sequence<=i->second.sequence)return reject("Old command ignored");
  if(c.action!="stop"&&c.action!="raise"&&c.action!="lower"&&c.action!="target")return reject("Invalid action");
  if(c.targetPercent<0||c.targetPercent>100)return reject("Invalid position");
  if(c.action!="stop"&&c.epoch!=generation)return reject("Movement invalidated by Stop");
  i->second.sequence=c.sequence;PeerDecision r{true,false,"",generation};cache.push_back({c.commandId,payload,r});if(cache.size()>32)cache.pop_front();return r;
 }
 void result(std::string id,bool accepted,std::string reason){for(auto& c:cache)if(c.id==id){c.reply={accepted,false,reason,generation};return;}}
 void stop(){++generation;}uint32_t epoch()const{return generation;}
 std::string bootNonce()const{return boot;}
 void resetSessions(){sessions.clear();cache.clear();++generation;}
};
