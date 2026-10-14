#pragma once
#include <cstdint>
struct Quadrature{
 uint8_t previous=0;
 void reset(uint8_t ab){previous=ab&3;}
 int update(uint8_t ab){
  ab&=3; uint8_t old=previous; previous=ab;
  if(old==ab || (old^ab)==3)return 0;
  return (old==0&&ab==1)||(old==1&&ab==3)||(old==3&&ab==2)||(old==2&&ab==0)?1:-1;
 }
};
