#include <cassert>
#include <cstdio>
#include "../ScheduledBlind/core/FleetDispatch.h"
int main(){FleetDispatch d;Device a{"a","A","a.local","key"};auto old=d.submit({a},"raise",100,0);auto work=d.nextWork(0);assert(work&&d.current(work->id));auto stop=d.submit({a},"stop",0,1);assert(stop);assert(!d.current(work->id));d.complete(work->id,{"Accepted",""},2);auto results=d.results(old);assert(results.size()==1&&results[0].state=="Rejected");FleetDispatch burst;std::vector<Device> devices;for(int i=0;i<8;++i)devices.push_back({std::to_string(i),"Blind","blind.local","key"});for(int i=0;i<8;++i)assert(burst.submit(devices,"stop",0,i));assert(burst.submit({devices[0]},"stop",0,9));
puts("PASS dispatch Stop: cancels active unsent movement and cannot later report it accepted");}
