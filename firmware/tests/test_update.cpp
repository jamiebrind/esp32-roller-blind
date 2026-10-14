#include <cassert>
#include <cstdio>
#include "../ScheduledBlind/core/UpdateCoordinator.h"
struct Flash:OtaPort {int begins=0,selects=0;bool hash=true,partial=false;bool begin(const FirmwareManifest&)override{++begins;return true;}size_t write(const uint8_t*,size_t n)override{return partial?n-1:n;}bool verify(const FirmwareManifest&)override{return hash;}bool selectCandidate()override{++selects;return true;}void abort()override{}};
FirmwareManifest valid(){return {"esp32c3-drv8871","2.0.0",1,300,std::string(64,'a')};}
int main(){Flash f;UpdateCoordinator u(f);assert(u.request(valid(),"one",0));assert(u.blocksMovement());assert(!u.stopped(1,true,false,1));assert(f.begins==0);assert(u.stopped(0,true,false,2));assert(f.begins==1);uint8_t data[300]={};
 assert(!u.chunk("two",data,300,3));assert(u.chunk("one",data,300,4));assert(u.finish("one",5));assert(f.selects==1&&u.blocksMovement());assert(!u.finish("one",6));
 Flash bad;bad.hash=false;UpdateCoordinator b(bad);assert(b.request(valid(),"one",0));b.stopped(0,true,false,0);b.chunk("one",data,300,1);assert(!b.finish("one",2));assert(bad.selects==0&&!b.blocksMovement());
 Flash timed;UpdateCoordinator timeout(timed);timeout.request(valid(),"one",0);timeout.stopped(0,true,false,0);timeout.tick(30000);assert(!timeout.blocksMovement()&&timed.selects==0);
 Flash save;UpdateCoordinator failed(save);failed.request(valid(),"one",0);assert(!failed.stopped(0,false,false,1));assert(save.begins==0&&!failed.blocksMovement());
 auto wrong=valid();wrong.hardware="esp32";assert(!failed.request(wrong,"one",2));wrong=valid();wrong.length=1966081;assert(!failed.request(wrong,"one",3));
 Flash shortf;UpdateCoordinator shortu(shortf);shortu.request(valid(),"one",0);shortu.stopped(0,true,false,1);assert(!shortu.finish("one",2)&&shortf.selects==0);
 Flash over;UpdateCoordinator excess(over);excess.request(valid(),"one",0);excess.stopped(0,true,false,1);assert(!excess.chunk("one",data,301,2)&&over.selects==0);
 puts("PASS update: applied stop, exclusive session, hash/size/target, timeout, commit ordering");}
