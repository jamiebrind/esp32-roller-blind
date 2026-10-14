#pragma once
#include <optional>
#include <cmath>
#include "UkTime.h"
struct SolarEvents{std::optional<int64_t> sunriseUtc,sunsetUtc;};
struct Solar{
 static double rad(double deg){return deg*3.14159265358979323846/180;}
 static std::optional<int64_t> event(Date utcDate,double lat,double lon,bool rise){
  double minute=rise?360:1080;
  int n=int(UkTime::days(utcDate)-UkTime::days({utcDate.year,1,1}))+1;
  bool leap=utcDate.year%4==0&&(utcDate.year%100!=0||utcDate.year%400==0);
  for(int i=0;i<4;++i){
   double g=2*3.14159265358979323846/(leap?366:365)*(n-1+(minute/60-12)/24);
   double eq=229.18*(0.000075+0.001868*cos(g)-0.032077*sin(g)-0.014615*cos(2*g)-0.040849*sin(2*g));
   double dec=0.006918-0.399912*cos(g)+0.070257*sin(g)-0.006758*cos(2*g)+0.000907*sin(2*g)-0.002697*cos(3*g)+0.00148*sin(3*g);
   double c=(cos(rad(90.833))/(cos(rad(lat))*cos(dec)))-tan(rad(lat))*tan(dec);
   if(c>1||c< -1)return {};
   double angle=acos(c)*180/3.14159265358979323846;
   minute=720-4*(lon+(rise?angle:-angle))-eq;
  }
  return UkTime::epoch(utcDate,0)+int64_t(llround(minute*60));
 }
 static SolarEvents events(Date d,double lat,double lon){
  SolarEvents out;if(!std::isfinite(lat)||!std::isfinite(lon)||lat< -90||lat>90||lon< -180||lon>180)return out;
  for(int shift=-1;shift<=1;++shift){auto date=UkTime::dateFromDays(UkTime::days(d)+shift);
   auto r=event(date,lat,lon,true),s=event(date,lat,lon,false);
   if(r){auto local=UkTime::fromUtc(*r).date;if(UkTime::days(local)==UkTime::days(d))out.sunriseUtc=r;}
   if(s){auto local=UkTime::fromUtc(*s).date;if(UkTime::days(local)==UkTime::days(d))out.sunsetUtc=s;}
  }return out;
 }
};
