#include <cassert>
#include <cstdio>
#include "../ScheduledBlind/core/AdminAuth.h"
int main(){int n=0;AdminAuth a([](auto p){return p=="longpassword";},[&]{return std::to_string(++n);});
 auto one=a.login("longpassword",0);assert(one.ok);auto two=a.login("longpassword",1);assert(two.ok&&one.token!=two.token);
 assert(a.authorised(one.token,2));a.logout(two.token);assert(!a.authorised(two.token,3));assert(a.authorised(one.token,4));
 for(int i=0;i<5;i++)assert(!a.login("wrong",10+i).ok);assert(!a.login("longpassword",15).ok);assert(a.authorised(one.token,16));assert(a.login("longpassword",60015).ok);
 assert(!a.authorised(one.token,1800000));
 AdminAuth wrap([](auto){return true;},[]{return "x";});auto w=wrap.login("p",0xfffffff0U);assert(w.ok);assert(wrap.authorised(w.token,20));assert(!wrap.authorised(w.token,1800000));
 wrap.tick(0xfffffff0U);assert(!wrap.authorised(w.token,20));puts("PASS admin auth: sessions, logout, expiry, lockout, monotonic rollover");}
