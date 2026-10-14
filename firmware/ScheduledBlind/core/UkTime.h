#pragma once
#include <cstdint>
struct Date{int year,month,day;};struct LocalTime{Date date;int hour,minute,second;bool bst;};
struct UkTime{
 static int64_t days(Date d){
  int y=d.year-(d.month<=2);int era=(y>=0?y:y-399)/400;unsigned yo=y-era*400;
  unsigned m=d.month+(d.month>2?-3:9);unsigned doy=(153*m+2)/5+d.day-1;
  return era*146097+yo*365+yo/4-yo/100+doy-719468;
 }
 static Date dateFromDays(int64_t z){
  z+=719468;int64_t era=(z>=0?z:z-146096)/146097;unsigned doe=z-era*146097;
  unsigned yo=(doe-doe/1460+doe/36524-doe/146096)/365;int y=yo+era*400;
  unsigned doy=doe-(365*yo+yo/4-yo/100),mp=(5*doy+2)/153;
  unsigned day=doy-(153*mp+2)/5+1;int month=mp+(mp<10?3:-9);y+=month<=2;return {y,month,int(day)};
 }
 static int weekday(Date d){int w=(days(d)+4)%7;return w<0?w+7:w;}
 static int lastSunday(int y,int m){return 31-weekday({y,m,31});}
 static int64_t epoch(Date d,int minute){return days(d)*86400+minute*60;}
 static bool isBst(int64_t t){auto d=dateFromDays(t/86400);return t>=epoch({d.year,3,lastSunday(d.year,3)},60)&&t<epoch({d.year,10,lastSunday(d.year,10)},60);}
 static LocalTime fromUtc(int64_t t){
  bool bst=isBst(t);int64_t local=t+(bst?3600:0);int64_t day=local/86400,sec=local%86400;if(sec<0){sec+=86400;--day;}
  return {dateFromDays(day),int(sec/3600),int(sec/60%60),int(sec%60),bst};
 }
 static int64_t toUtc(Date d,int minute){
  if(d.month==3&&d.day==lastSunday(d.year,3)&&minute>=60&&minute<120)return epoch(d,60);
  int64_t candidate=epoch(d,minute)-3600;
  if(isBst(candidate))return candidate;return epoch(d,minute);
 }
};
struct ClockState{
 int64_t utc=0;mutable uint32_t checked=0;mutable uint64_t age=0;bool synced=false;
 void sync(int64_t value,uint32_t now){utc=value;checked=now;age=0;synced=true;}
 uint64_t lastSyncAgeMs(uint32_t now)const{age+=uint32_t(now-checked);checked=now;return age;}
 bool valid(uint32_t now)const{return synced&&lastSyncAgeMs(now)<=86400000U;}
 int64_t utcNow(uint32_t now)const{return utc+lastSyncAgeMs(now)/1000;}
};
