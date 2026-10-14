#include <cassert>
#include <cstdio>
#include "../ScheduledBlind/core/Motion.h"
#include "../ScheduledBlind/core/Quadrature.h"
int main(){
  Motion m; assert(m.status().direction==0); assert(!m.moveTo(100,0));
  assert(m.jog(1,0)); m.tick(599,0); assert(m.status().direction==1);
  m.tick(600,0); assert(m.status().direction==0);
 assert(!m.jog(1,950)); // An expired heartbeat cannot start a fresh jog.
 m.stop(960);
  assert(m.jog(-1,1000)); m.stop(1010);assert(!m.jog(-1,1100));assert(m.jog(-1,1210));m.stop(1220);
  m.beginCalibration(); m.tick(1000,100); assert(m.saveUpper(100));
  assert(!m.saveLower(100)); assert(m.saveLower(1100)); assert(m.endCalibration());
  assert(m.status().calibrated && m.status().positionKnown);
  assert(m.status().count==1000);
  assert(m.moveTo(100,1430)); m.tick(1440,600); assert(m.status().direction==1);
  m.tick(1500,100); assert(m.status().direction==0 && m.status().count==0);
  assert(!m.moveTo(101,1800)); assert(m.moveTo(0,1800));
  m.tick(1900,1100); assert(m.status().direction==0);
  assert(m.moveTo(100,2200)); m.tick(2300,1200); assert(!m.status().fault.empty());
  assert(m.status().direction==0); m.resetFault();
  m.beginCalibration(); assert(m.saveUpper(1200)); assert(!m.status().calibrated);
  Motion stalled; assert(stalled.jog(1,0));
  for(unsigned t=100;t<1000;t+=100){stalled.jog(1,t);stalled.tick(t,0);}
  stalled.tick(1000,0); assert(stalled.status().direction==0 && !stalled.status().fault.empty());
  Motion mismatch;mismatch.restore({400,true,false,1},1000,1,2);assert(!mismatch.status().positionKnown);
  PositionRecord dirty{500,true,true}; m.restore(dirty,1000,1); assert(!m.status().positionKnown); assert(!m.moveTo(100,4000));
 Motion calibration;calibration.beginCalibration();calibration.saveUpper(0);calibration.saveLower(500);
 assert(!calibration.status().positionKnown); // Not trusted until calibration is committed.
 Motion slow;slow.beginCalibration();slow.saveUpper(0);slow.saveLower(1000);slow.endCalibration();
 assert(slow.moveTo(100,0));slow.tick(950,999);slow.tick(1900,998);slow.tick(2000,998);assert(!slow.status().fault.empty());
  Motion overflow; assert(overflow.jog(1,0xffffff00U)); overflow.tick(0x158U,0); assert(overflow.status().direction==0);
  Quadrature q; q.reset(0); assert(q.update(1)==1); assert(q.update(3)==1); assert(q.update(2)==1); assert(q.update(0)==1); assert(q.update(3)==0);
  puts("PASS motion: calibration, limits, reversal, heartbeat, encoder fault, interrupted restore, rollover, quadrature");
}
