#pragma once
#include <cstdint>
struct WifiRetry {
 bool paused=false,started=false;uint32_t last=0;
 void attempted(uint32_t now){started=true;last=now;}
 bool due(bool connected,uint32_t now)const{return !connected&&!paused&&(!started||uint32_t(now-last)>=30000);}
};
