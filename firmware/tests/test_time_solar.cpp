#include <cassert>
#include <cstdio>
#include <cstdlib>
#include "../ScheduledBlind/core/UkTime.h"
#include "../ScheduledBlind/core/Solar.h"
int main(){
 auto march=UkTime::epoch({2026,3,29},59);
 auto before=UkTime::fromUtc(march+59); assert(before.hour==0 && before.minute==59 && !before.bst);
 auto after=UkTime::fromUtc(march+60); assert(after.hour==2 && after.minute==0 && after.bst);
 assert(UkTime::toUtc({2026,3,29},90)==UkTime::epoch({2026,3,29},60));
 auto oct=UkTime::epoch({2026,10,25},60);auto fall=UkTime::fromUtc(oct);assert(fall.hour==1&&!fall.bst);
 assert(UkTime::toUtc({2026,10,25},90)==UkTime::epoch({2026,10,25},30));
 assert(UkTime::fromUtc(UkTime::epoch({2028,2,29},0)).date.day==29);
 auto summer=Solar::events({2026,6,21},51.5074,-0.1278);
 assert(summer.sunriseUtc && summer.sunsetUtc);
 assert(std::llabs(*summer.sunriseUtc-UkTime::epoch({2026,6,21},223))<300);
 assert(std::llabs(*summer.sunsetUtc-UkTime::epoch({2026,6,21},1222))<300);
 auto winter=Solar::events({2026,12,21},51.5074,-0.1278);assert(winter.sunriseUtc&&winter.sunsetUtc);
 assert(std::llabs(*winter.sunriseUtc-UkTime::epoch({2026,12,21},484))<300);
 assert(std::llabs(*winter.sunsetUtc-UkTime::epoch({2026,12,21},953))<300);
 auto east=Solar::events({2026,6,21},35.7,139.7);assert(east.sunriseUtc&&UkTime::days(UkTime::fromUtc(*east.sunriseUtc).date)==UkTime::days({2026,6,21}));
 auto polar=Solar::events({2026,6,21},89,0);assert(!polar.sunriseUtc&&!polar.sunsetUtc);
 ClockState clock;assert(!clock.valid(0));clock.sync(100000,0xffffff00U);assert(clock.valid(0x100U));assert(clock.utcNow(0x100U)==100000);
 assert(!clock.valid(uint32_t(0xffffff00U+86400001U)));
 ClockState longOutage;longOutage.sync(100000,0);
 for(uint32_t t: {1000000000U,2000000000U,3000000000U,4000000000U})assert(!longOutage.valid(t));
 assert(!longOutage.valid(30000U)); // Still stale after millis wraps at 49.7 days.
 puts("PASS time/solar: UK DST gap/fold, leap day, independent USNO solar fixtures, polar events, holdover/rollover");
}
