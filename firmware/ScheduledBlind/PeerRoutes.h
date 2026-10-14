#pragma once
bool peerKeyValid(){xSemaphoreTake(fleetMutex,portMAX_DELAY);auto key=fleetStorage.pairingKey();xSemaphoreGive(fleetMutex);return secureEqual(web.header("X-Peer-Key").c_str(),key);}
bool peerBody(JsonDocument& d){if(!peerKeyValid()){replyError("Pairing key rejected",403);return false;}String raw=web.arg("plain");if(raw.length()>1024||deserializeJson(d,raw)||!d.is<JsonObject>()){replyError("Invalid peer command");return false;}return true;}
PeerResult applyPeerMotionLocked(const std::string& action,int percent,uint32_t expectedEpoch=0){
 auto s=motion.status();PeerResult result{"Rejected","Movement rejected: check limits, position or fault"};
 if(action=="stop"){commands.stop();peerProtocol->stop();jogOwner.clear();motion.stop(millis());result={"Accepted",""};}
 else if(expectedEpoch&&expectedEpoch!=commands.epoch())result={"Rejected","Movement invalidated by local Stop"};
 else if(maintenance)result={"Updating","Firmware update in progress"};
 else if(motion.inCalibration()||!jogOwner.empty()&&s.direction)result={"Rejected","Local control has priority"};
 else if(motion.moveTo(action=="raise"?100:action=="lower"?0:percent,millis()))result={"Accepted",""};
 return result;
}
PeerResult applyPeerMotion(const std::string& action,int percent,uint32_t expectedEpoch=0){
 xSemaphoreTake(stateMutex,portMAX_DELAY);auto result=applyPeerMotionLocked(action,percent,expectedEpoch);xSemaphoreGive(stateMutex);return result;
}
void peerStatusJson(JsonDocument& d){xSemaphoreTake(stateMutex,portMAX_DELAY);auto s=motion.status();d["deviceId"]=deviceId;d["version"]=FIRMWARE_VERSION;d["protocolVersion"]=1;d["bootNonce"]=peerProtocol->bootNonce();d["atUpper"]=motion.atUpper();d["atLower"]=motion.atLower();d["positionKnown"]=s.positionKnown;d["positionPercent"]=motion.percent();d["direction"]=s.direction;d["fault"]=s.fault;d["updating"]=maintenance;d["epoch"]=peerProtocol->epoch();xSemaphoreGive(stateMutex);d["ok"]=true;}
void registerPeerRoutes(){
 web.on("/api/peer/info",HTTP_GET,[]{if(!peerKeyValid()){replyError("Pairing key rejected",403);return;}JsonDocument d;peerStatusJson(d);d["name"]=localName;String text;serializeJson(d,text);web.send(200,"application/json",text);});
 web.on("/api/peer/status",HTTP_GET,[]{if(!peerKeyValid()){replyError("Pairing key rejected",403);return;}JsonDocument d;peerStatusJson(d);String text;serializeJson(d,text);web.send(200,"application/json",text);});
 web.on("/api/peer/session",HTTP_POST,[]{JsonDocument request;if(!peerBody(request))return;auto nonce=randomHex();xSemaphoreTake(stateMutex,portMAX_DELAY);bool ok=peerProtocol->open(nonce,millis());uint32_t epoch=peerProtocol->epoch();xSemaphoreGive(stateMutex);if(!ok){replyError("Too many peer sessions",429);return;}JsonDocument d;d["ok"]=true;d["deviceId"]=deviceId;d["nonce"]=nonce;d["epoch"]=epoch;String text;serializeJson(d,text);web.send(200,"application/json",text);});
 web.on("/api/peer/move",HTTP_POST,[]{JsonDocument d;if(!peerBody(d))return;
  if(!d["commandId"].is<const char*>()||!d["nonce"].is<const char*>()||!d["sequence"].is<uint32_t>()||!d["epoch"].is<uint32_t>()||!d["action"].is<const char*>()||!d["targetPercent"].is<int>()){replyError("Invalid typed command");return;}
  PeerCommand c{d["commandId"].as<std::string>(),d["nonce"].as<std::string>(),d["sequence"].as<uint32_t>(),d["epoch"].as<uint32_t>(),d["action"].as<std::string>(),d["targetPercent"].as<int>()};
  xSemaphoreTake(stateMutex,portMAX_DELAY);auto decision=peerProtocol->accept(c,true,millis());
  PeerResult result{decision.accepted?"Accepted":"Rejected",decision.reason};
  if(decision.accepted&&!decision.duplicate){result=applyPeerMotionLocked(c.action,c.targetPercent);peerProtocol->result(c.commandId,result.state=="Accepted",result.reason);decision.epoch=peerProtocol->epoch();}
  xSemaphoreGive(stateMutex);
  JsonDocument out;out["ok"]=true;out["deviceId"]=deviceId;out["commandId"]=c.commandId;out["accepted"]=result.state=="Accepted";out["state"]=result.state;out["reason"]=result.reason;out["epoch"]=decision.epoch;String text;serializeJson(out,text);web.send(200,"application/json",text);
 });
}
