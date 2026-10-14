#include <cassert>
#include <cstdio>
#include "../ScheduledBlind/core/UpdateCoordinator.h"
struct Flash:OtaPort{bool begin(const FirmwareManifest&)override{return true;}size_t write(const uint8_t*,size_t n)override{return n;}bool verify(const FirmwareManifest&)override{return true;}bool selectCandidate()override{return true;}void abort()override{}};
int main(){Flash f;UpdateCoordinator u(f);FirmwareManifest m{"esp32c3-drv8871","2.0.1",1,300,std::string(64,'a')};uint8_t data[300]={};assert(u.request(m,"owner",0));assert(u.stopped(0,true,false,1));assert(u.chunk("owner",data,300,2));assert(u.finish("owner",3));u.abort("HTTP disconnected after commit");assert(u.blocksMovement());assert(!u.rebootDue(1502));assert(u.rebootDue(1503));puts("PASS OTA final disconnect: verified candidate reboots even when final HTTP response is lost");}
