#include <cassert>
#include <cstdio>
#include "../ScheduledBlind/core/SenderSession.h"
int main(){SenderSession s{"nonce",1,1,100,"boot1"};assert(senderReusable(s,"boot1",101));assert(!senderReusable(s,"boot2",101));assert(!senderReusable(s,"boot1",1700100));s.started=0xffffff00U;assert(senderReusable(s,"boot1",0x100U));puts("PASS sender: reuse only same boot and live session, rollover");}
