#include <cassert>
#include <cstdio>
#include "../ScheduledBlind/core/CommandGate.h"
int main(){
 CommandGate gate;gate.issue("pageA");assert(gate.accept("pageA",1));
 gate.issue("pageB");assert(gate.accept("pageA",2)); // A's Stop still authorised.
 auto epoch=gate.epoch();gate.stop();assert(!gate.canMove(epoch));
 assert(gate.canMove(gate.epoch()));assert(!gate.accept("pageA",2));
 assert(gate.accept("pageB",1));puts("PASS command gate: independent pages, replay rejection, Stop invalidates delayed motion");
}
