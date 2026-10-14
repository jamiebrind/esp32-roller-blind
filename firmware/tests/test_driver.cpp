#include <cassert>
#include <vector>
#include <cstdio>
#include "../ScheduledBlind/Drv8871.h"
int main(){std::vector<std::pair<int,int>> out;auto write=[&](int a,int b){out.emplace_back(a,b);};DriverOutput d;d.apply(1,200,write);assert((out.back()==std::pair<int,int>(200,0)));d.apply(-1,100,write);assert(out[out.size()-2]==std::make_pair(0,0));assert(out.back()==std::make_pair(0,100));d.apply(0,0,write);assert(out.back()==std::make_pair(0,0));puts("PASS DRV8871: forward/reverse PWM mapping, disable before reversal, Stop");}
