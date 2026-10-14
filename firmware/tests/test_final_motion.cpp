#include <cassert>
#include <cstdio>
#include "../ScheduledBlind/core/Motion.h"
#include "../ScheduledBlind/core/Settings.h"
int main(){
 Motion m;m.beginCalibration();m.saveUpper(0);m.saveLower(1000000);m.endCalibration();
 assert(!m.moveTo(0,0)); // Lower at lower endpoint must be rejected.
 m.referenceUpper(0);assert(!m.moveTo(100,0));assert(!m.jog(1,0));
 assert(m.moveTo(0,1000));for(unsigned n=1000;n<60000;n+=250)m.tick(1000+n,n*10);
 assert(m.status().direction==-1);m.tick(61000,600000);assert(m.status().direction==0&&!m.status().fault.empty());
 Motion jog;jog.beginCalibration();assert(jog.newJog(-1,0));
 for(unsigned n=250;n<60000;n+=250){assert(jog.jog(-1,n));jog.tick(n,n);}
 assert(jog.status().direction==-1);jog.tick(60000,60000);assert(jog.status().direction==0&&!jog.status().fault.empty());
 assert(!jog.inCalibration()&&!jog.status().calibrated);jog.resetFault();assert(!jog.endCalibration());
 Settings s;assert(s.maxRuntime==60000);s.maxRuntime=61000;assert(!SettingsValidator::validate(s).empty());
 puts("PASS final motion: reject reached endpoint, 60-second auto/jog timeout, calibration fault abort, capped settings");
}
