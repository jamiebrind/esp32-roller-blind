#pragma once
#include <string>
#include <cstdint>
struct SenderSession {std::string nonce;uint32_t sequence=0,epoch=1,started=0;std::string boot;};
inline bool senderReusable(const SenderSession& s,const std::string& boot,uint32_t now){return !s.nonce.empty()&&!boot.empty()&&s.boot==boot&&uint32_t(now-s.started)<1700000;}
