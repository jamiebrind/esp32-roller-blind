#pragma once
#include "Motion.h"
#include "Settings.h"
inline void restoreBootPosition(Motion& motion,PositionRecord saved,const Settings& settings){
 // Trust only a committed stopped record for this calibration. A moving marker,
 // invalid/missing record or changed calibration still leaves position unknown.
 // Assumption: the chain has not moved manually while the controller was off.
 motion.restore(saved,settings.span,settings.encoderSign,settings.calibrationVersion);
}
