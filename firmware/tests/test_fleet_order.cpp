#include <cassert>
#include <cstdio>
#include "../ScheduledBlind/core/CommandGate.h"
int main(){CommandGate g;g.issue("page");auto epoch=g.epoch();assert(g.acceptMovement("page",1,epoch));g.stop();assert(!g.acceptMovement("page",2,epoch));assert(g.acceptMovement("page",3,g.epoch()));assert(!g.acceptMovement("page",2,g.epoch()));puts("PASS fleet ordering: delayed movement rejected after Stop, fresh movement allowed");}
