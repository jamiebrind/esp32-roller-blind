#pragma once
#include <cstdint>
#include <string>
#include <algorithm>
struct PositionRecord{int64_t count;bool valid,moving;uint32_t calibrationVersion=0;};
struct MotionStatus{int64_t count=0,target=0;int direction=0;bool calibrated=false,positionKnown=false;std::string fault;};
class Motion{
 MotionStatus s; int64_t raw=0,upper=0,travel=0; int sign=1;
 uint32_t started=0,heartbeat=0,lastMotion=0,lastStop=0;
 bool expiredJog=false;uint32_t progressStart=0;int64_t progressCount=0;
 bool stoppedBefore=false,calibration=false,upperSet=false,automatic=false;
 bool start(int dir,uint32_t now){
  if(!s.fault.empty() || (dir!=1&&dir!=-1))return false;
  if(s.direction && s.direction!=dir){stop(now);return false;}
  if(!s.direction){if(stoppedBefore && uint32_t(now-lastStop)<200)return false;s.direction=dir;started=lastMotion=progressStart=now;progressCount=s.count;}
  heartbeat=now;return true;
 }
 void fail(const char* reason,uint32_t now){stop(now);if(calibration){invalidateCalibration();upperSet=false;}s.fault=reason;s.positionKnown=false;}
public:
 uint32_t maxRuntime=60000,stallMs=1000,minProgressCounts=4;
 MotionStatus status()const{return s;}
 int64_t span()const{return travel;} int encoderSign()const{return sign;}
 bool inCalibration()const{return calibration;}
 int percent()const{return travel>0?int(std::clamp<int64_t>((travel-s.count)*100/travel,0,100)):0;}
 bool atUpper()const{return s.calibrated&&s.positionKnown&&!calibration&&s.count<=0;}
 bool atLower()const{return s.calibrated&&s.positionKnown&&!calibration&&s.count>=travel;}
 bool newJog(int dir,uint32_t now){expiredJog=false;return jog(dir,now);}
 bool jog(int dir,uint32_t now){if(expiredJog||(dir==1&&atUpper())||(dir==-1&&atLower()))return false;automatic=false;return start(dir,now);}
 bool moveTo(int percent,uint32_t now){
  if(percent<0||percent>100||!s.calibrated||!s.positionKnown||calibration||!s.fault.empty())return false;
  if((percent==100&&atUpper())||(percent==0&&atLower()))return false;
  int64_t wanted=travel*(100-percent)/100;
  if(wanted==s.count)return false;
  if(!start(wanted<s.count?1:-1,now))return false;
  s.target=wanted;automatic=true;return true;
 }
 void stop(uint32_t now){expiredJog=false;if(s.direction){lastStop=now;stoppedBefore=true;}s.direction=0;automatic=false;}
 void tick(uint32_t now,int64_t encoderCount){
  int64_t before=s.count;raw=encoderCount;s.count=(raw-upper)*sign;
  if(!s.direction)return;
  int64_t delta=s.count-before;
  if(delta){lastMotion=now;if(!calibration && s.positionKnown && delta*s.direction>0){fail("Encoder direction disagrees with movement",now);return;}}
  if(!automatic && uint32_t(now-heartbeat)>=600){stop(now);expiredJog=true;return;}
  if(uint32_t(now-started)>=std::min<uint32_t>(maxRuntime,60000)){fail("Maximum run time reached",now);return;}
  if(uint32_t(now-started)>=1000 && uint32_t(now-lastMotion)>=stallMs){fail("No encoder movement: check motor and wiring",now);return;}
  if(uint32_t(now-progressStart)>=2000){if(std::llabs(s.count-progressCount)<minProgressCounts){fail("Encoder progress too slow: check load and wiring",now);return;}progressStart=now;progressCount=s.count;}
  if(!calibration && s.calibrated && s.positionKnown){
   if((s.direction==1&&s.count<=0)||(s.direction==-1&&s.count>=travel)||
      (automatic&&((s.direction==1&&s.count<=s.target)||(s.direction==-1&&s.count>=s.target))))stop(now);
  }
 }
 void beginCalibration(){stop(lastStop);invalidateCalibration();calibration=true;upperSet=false;s.positionKnown=false;s.fault.clear();}
 bool saveUpper(int64_t count){if(!calibration||s.direction)return false;upper=count;raw=count;s.count=0;upperSet=true;s.calibrated=false;travel=0;return true;}
 bool saveLower(int64_t count){
  if(!calibration||!upperSet||s.direction||count==upper)return false;
  sign=count>upper?1:-1;travel=(count-upper)*sign;raw=count;s.count=travel;s.calibrated=true;s.positionKnown=false;return true;
 }
 bool endCalibration(){if(!calibration||s.direction||!s.calibrated||!s.fault.empty())return false;calibration=false;s.positionKnown=true;return true;}
 void resetFault(){if(!s.direction)s.fault.clear();}
 void invalidatePosition(){s.positionKnown=false;}
 void invalidateCalibration(){s.calibrated=s.positionKnown=false;travel=0;calibration=false;}
 void latchFault(const char* reason,uint32_t now){fail(reason,now);}
 void referenceUpper(int64_t count){if(!s.direction&&s.calibrated){upper=raw=count;s.count=0;s.positionKnown=true;s.fault.clear();}}
 void restore(PositionRecord p,int64_t span,int encoderSign,uint32_t calibrationVersion=0){
  travel=span;sign=encoderSign==-1?-1:1;upper=-p.count*sign;raw=0;s.count=p.count;
  s.calibrated=span>0;s.positionKnown=p.valid&&!p.moving&&s.calibrated&&p.calibrationVersion==calibrationVersion;
 }
};
