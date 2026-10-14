#pragma once
#include <cstdint>
struct DriverOutput{
 int last=0;
 template<class Write>void apply(int direction,uint8_t duty,Write write){
  if(direction!=last)write(0,0);
  write(direction==1?duty:0,direction==-1?duty:0);last=direction;
 }
};
