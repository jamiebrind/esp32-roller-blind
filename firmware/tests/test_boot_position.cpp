#include <cassert>
#include <cstdio>
#include "../ScheduledBlind/core/BootPosition.h"
int main(){
 Settings s;s.span=1000;s.encoderSign=1;s.calibrationVersion=4;
 Motion clean;restoreBootPosition(clean,{400,true,false,4},s);
 assert(clean.status().positionKnown&&clean.status().calibrated);
 assert(clean.status().direction==0&&clean.status().count==400&&clean.percent()==60);
 clean.tick(100,0);assert(clean.status().count==400); // Fresh encoder origin preserves saved position.
 Motion down;restoreBootPosition(down,{1000,true,false,4},s);assert(down.atLower());assert(!down.moveTo(0,0));
 Motion up;restoreBootPosition(up,{0,true,false,4},s);assert(up.atUpper());assert(!up.moveTo(100,0));
 Motion interrupted;restoreBootPosition(interrupted,{400,true,true,4},s);assert(!interrupted.status().positionKnown);assert(!interrupted.moveTo(100,0));
 Motion invalid;restoreBootPosition(invalid,{400,false,false,4},s);assert(!invalid.status().positionKnown);
 Motion changed;restoreBootPosition(changed,{400,true,false,3},s);assert(!changed.status().positionKnown);
 Settings uncalibrated=s;uncalibrated.span=0;Motion noLimits;restoreBootPosition(noLimits,{0,true,false,4},uncalibrated);assert(!noLimits.status().positionKnown);
 s.encoderSign=-1;Motion reversed;restoreBootPosition(reversed,{400,true,false,4},s);reversed.tick(100,0);assert(reversed.status().positionKnown&&reversed.status().count==400);
 puts("PASS boot position: clean saved state, endpoints, fresh encoder origin, interrupted/invalid/calibration mismatch");
}
