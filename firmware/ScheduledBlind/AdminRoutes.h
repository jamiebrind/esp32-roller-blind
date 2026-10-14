#pragma once
bool requestOrigin(){if(web.header("X-Blind-Request")!="1")return false;String origin=web.header("Origin");return origin.isEmpty()||origin=="http://"+web.hostHeader();}
bool requireAdmin(){if(!adminAuth.authorised(web.header("X-Blind-Auth").c_str(),millis())){replyError("Please log in",401);return false;}return true;}
void registerAdminRoutes(){
 web.on("/api/login",HTTP_POST,[]{if(!requestOrigin()){replyError("Request origin rejected",403);return;}JsonDocument d;String raw=web.arg("plain");if(raw.length()>256||deserializeJson(d,raw)||!d["password"].is<const char*>()){replyError("Invalid login");return;}auto result=adminAuth.login(d["password"].as<std::string>(),millis());if(!result.ok){replyError(result.error,401);return;}JsonDocument out;out["ok"]=true;out["token"]=result.token;out["expiresSeconds"]=1800;String text;serializeJson(out,text);web.sendHeader("Cache-Control","no-store");web.send(200,"application/json",text);});
 web.on("/api/logout",HTTP_POST,[]{if(!requestOrigin()||!requireAdmin())return;adminAuth.logout(web.header("X-Blind-Auth").c_str());xSemaphoreTake(stateMutex,portMAX_DELAY);motion.stop(millis());commands.stop();if(peerProtocol)peerProtocol->stop();jogOwner.clear();xSemaphoreGive(stateMutex);replyOK();});
}
