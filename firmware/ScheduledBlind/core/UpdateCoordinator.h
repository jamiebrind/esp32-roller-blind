#pragma once
#include <string>
#include <cstdint>
#include <cstddef>
#include <algorithm>
struct FirmwareManifest {std::string hardware,version;uint32_t settingsSchema;size_t length;std::string sha256;};
struct OtaPort {virtual bool begin(const FirmwareManifest&)=0;virtual size_t write(const uint8_t*,size_t)=0;virtual bool verify(const FirmwareManifest&)=0;virtual bool selectCandidate()=0;virtual void abort()=0;virtual ~OtaPort()=default;};
enum class UpdateState {Idle,WaitingForStop,Receiving,Failed,RebootPending};
class UpdateCoordinator {
 OtaPort& port;FirmwareManifest manifest;std::string session,failure;UpdateState phase=UpdateState::Idle;size_t bytes=0;uint32_t last=0;
public:
 explicit UpdateCoordinator(OtaPort& p):port(p){}
 bool request(FirmwareManifest m,std::string token,uint32_t now){
  if(blocksMovement())return false;
  if(token.empty()||m.hardware!="esp32c3-drv8871"||m.version.empty()||m.version.size()>32||m.settingsSchema!=1||m.length<288||m.length>1966080||m.sha256.size()!=64||!std::all_of(m.sha256.begin(),m.sha256.end(),[](char c){return(c>='0'&&c<='9')||(c>='a'&&c<='f');})){failure="Incompatible firmware manifest";return false;}
  manifest=m;session=token;bytes=0;failure.clear();phase=UpdateState::WaitingForStop;last=now;return true;
 }
 bool stopped(int applied,bool saved,bool calibrating,uint32_t now){if(phase!=UpdateState::WaitingForStop)return false;if(calibrating){abort("Finish calibration first");return false;}if(applied)return false;if(!saved){abort("Cannot persist stopped position");return false;}if(!port.begin(manifest)){abort("Cannot open inactive OTA slot");return false;}phase=UpdateState::Receiving;last=now;return true;}
 bool chunk(std::string token,const uint8_t* data,size_t n,uint32_t now){if(phase!=UpdateState::Receiving||token!=session)return false;if(n>manifest.length-bytes){abort("Firmware exceeds declared length");return false;}if(port.write(data,n)!=n){abort("Firmware write failed");return false;}bytes+=n;last=now;return true;}
 bool finish(std::string token,uint32_t now){if(phase!=UpdateState::Receiving||token!=session)return false;if(bytes!=manifest.length||!port.verify(manifest)){abort("Firmware length, hash or image verification failed");return false;}if(!port.selectCandidate()){abort("Firmware commit failed");return false;}phase=UpdateState::RebootPending;last=now;return true;}
 void abort(std::string reason){if(phase==UpdateState::RebootPending)return;port.abort();phase=UpdateState::Failed;failure=reason;}
 void tick(uint32_t now){if((phase==UpdateState::WaitingForStop||phase==UpdateState::Receiving)&&uint32_t(now-last)>=30000)abort("Firmware transfer timed out");}
 bool rebootDue(uint32_t now)const{return phase==UpdateState::RebootPending&&uint32_t(now-last)>=1500;}
 bool blocksMovement()const{return phase==UpdateState::WaitingForStop||phase==UpdateState::Receiving||phase==UpdateState::RebootPending;}
 UpdateState state()const{return phase;}size_t received()const{return bytes;}std::string error()const{return failure;}
};
