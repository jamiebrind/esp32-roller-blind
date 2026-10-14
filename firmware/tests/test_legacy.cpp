#include <cassert>
#include <cstdio>
#include "../ScheduledBlind/core/LegacyLimits.h"
int main(){JsonDocument inner,outer;inner["version"]=1;inner["generation"]=2;inner["span"]=3456;inner["sign"]=-1;inner["count"]=100;inner["moving"]=false;inner["valid"]=true;std::string payload,record;serializeJson(inner,payload);outer["payload"]=payload;outer["checksum"]=checksum(payload);serializeJson(outer,record);
 auto s=legacyLimits(record);assert(s&&s->span==3456&&s->encoderSign==-1&&s->calibrationVersion==2&&!s->schedulesEnabled&&s->maxRuntime==60000);
 outer["checksum"]=0;record.clear();serializeJson(outer,record);assert(!legacyLimits(record));
 assert(!legacyLimits("{}"));puts("PASS migration: checksummed r1 span/sign import, no auto schedules, malformed record rejected");}
