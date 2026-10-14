#include <cassert>
#include <cstdio>
#include "../ScheduledBlind/core/FleetRegistry.h"
int main(){FleetRegistry r({"local","Local","local.local",""});assert(r.add({"a","Room A","blind-a.local","key"}).empty());assert(!r.add({"a","Dup","other.local","key"}).empty());assert(!r.add({"b","Other","blind-a.local","key"}).empty());assert(!r.saveGroup({"g","Empty",{}}).empty());assert(!r.saveGroup({"g","Wrong",{"x"}}).empty());assert(r.saveGroup({"g","Rooms",{"local","a"}}).empty());auto snap=r.snapshot("g");assert(snap.size()==2);r.remove("a");assert(snap.size()==2&&r.snapshot("g").size()==1);
 assert(!FleetRegistry::endpointValid("http://x/path"));assert(!FleetRegistry::endpointValid("127.0.0.1"));assert(!FleetRegistry::endpointValid("224.0.0.1"));assert(!FleetRegistry::endpointValid("0.0.0.0"));assert(FleetRegistry::endpointValid("192.168.1.42:80"));
 for(int i=0;i<7;i++)assert(r.add({std::to_string(i),"B","blind-"+std::to_string(i)+".local","k"}).empty());assert(!r.add({"nine","B","extra.local","k"}).empty());puts("PASS fleet registry: limits, duplicate identity/address, memberships, snapshots, endpoint validation");}
