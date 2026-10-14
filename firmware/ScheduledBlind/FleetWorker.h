#pragma once
#include <HTTPClient.h>
#include <WiFiClient.h>
struct HttpResult {int status;std::string error;};
HttpResult peerRequest(Device device,const char* path,JsonDocument& response,const JsonDocument* request=nullptr){
 WiFiClient client;HTTPClient http;if(!http.begin(client,("http://"+device.endpoint+path).c_str()))return {0,"Invalid peer endpoint"};http.setConnectTimeout(2000);http.setTimeout(2000);http.setReuse(false);http.addHeader("X-Peer-Key",device.pairingKey.c_str());int status;
 if(request){http.addHeader("Content-Type","application/json");String payload;serializeJson(*request,payload);status=http.POST(payload);}else status=http.GET();
 if(status!=200){http.end();return {status,status==HTTPC_ERROR_READ_TIMEOUT?"Timed out":"Peer unreachable or authentication rejected"};}
 int size=http.getSize();if(size<1||size>8192){http.end();return {0,"Invalid bounded peer response"};}
 std::vector<char> bytes(size);size_t got=http.getStreamPtr()->readBytes(bytes.data(),size);http.end();
 if(got!=size||deserializeJson(response,bytes.data(),size)||!response["ok"].is<bool>()||!response["ok"].as<bool>())return {0,"Invalid peer response"};
 if(!response["deviceId"].is<const char*>()||(!device.id.empty()&&response["deviceId"].as<std::string>()!=device.id))return {0,"Peer identity changed"};return {status,""};
}
void recordPeerStatus(Device device,JsonDocument& d){PeerStatus status;status.epoch=d["epoch"]|0U;status.boot=d["bootNonce"]|"";status.seen=millis();status.contacted=true;status.atUpper=d["atUpper"]|false;status.atLower=d["atLower"]|false;status.positionKnown=d["positionKnown"]|false;status.percent=d["positionPercent"]|0;status.direction=d["direction"]|0;status.updating=d["updating"]|false;status.fault=d["fault"]|"";status.version=d["version"]|"";status.state="Online";xSemaphoreTake(fleetMutex,portMAX_DELAY);fleetStatuses[device.id]=status;xSemaphoreGive(fleetMutex);}
bool workCurrent(PeerWork work){xSemaphoreTake(fleetMutex,portMAX_DELAY);bool current=fleetDispatch.current(work.id);xSemaphoreGive(fleetMutex);return current;}
PeerResult doPeerWork(PeerWork work){
 if(!workCurrent(work))return {"Rejected","Superseded by Stop or maintenance"};
 xSemaphoreTake(stateMutex,portMAX_DELAY);bool blocked=maintenance;xSemaphoreGive(stateMutex);if(blocked&&work.action!="stop")return {"Rejected","Dashboard maintenance"};
 if(work.device.id==deviceId){if(!workCurrent(work))return {"Rejected","Superseded by Stop"};if(work.poll)return {"Accepted",""};return applyPeerMotion(work.action,work.percent,work.action=="stop"?0:work.device.epoch);}
 JsonDocument status;auto info=peerRequest(work.device,"/api/peer/status",status);if(!info.error.empty())return {info.status==HTTPC_ERROR_READ_TIMEOUT?"Timeout":"Offline",info.error};recordPeerStatus(work.device,status);if(work.poll)return {"Accepted",""};
 if((status["protocolVersion"]|0)!=1)return {"Rejected","Unsupported peer protocol"};
 if(work.action!="stop"&&(status["updating"]|false))return {"Updating","Firmware update in progress"};
 SenderSession sender;xSemaphoreTake(fleetMutex,portMAX_DELAY);sender=senderSessions[work.device.id];xSemaphoreGive(fleetMutex);
 if(!senderReusable(sender,std::string(status["bootNonce"]|""),millis())){JsonDocument request,response;request.to<JsonObject>();auto result=peerRequest(work.device,"/api/peer/session",response,&request);if(!result.error.empty())return {"Offline",result.error};if(!response["nonce"].is<const char*>()||!response["epoch"].is<uint32_t>())return {"Rejected","Invalid peer session"};sender={response["nonce"].as<std::string>(),0,response["epoch"].as<uint32_t>(),millis(),std::string(status["bootNonce"]|"")};}
 if(work.action!="stop"&&(!work.device.epoch||work.device.expectedBoot!=std::string(status["bootNonce"]|"")||work.device.epoch!=(status["epoch"]|0U)))return {"Rejected","Device stopped or restarted since command; send a new command"};
 sender.epoch=work.action=="stop"?(status["epoch"]|sender.epoch):work.device.epoch;
 JsonDocument request,response;request["commandId"]=randomHex();request["nonce"]=sender.nonce;request["sequence"]=++sender.sequence;request["epoch"]=sender.epoch;request["action"]=work.action;request["targetPercent"]=work.percent;
 if(!workCurrent(work))return {"Rejected","Superseded by Stop or maintenance"};
 auto result=peerRequest(work.device,"/api/peer/move",response,&request);
 if(!result.error.empty()){xSemaphoreTake(fleetMutex,portMAX_DELAY);senderSessions.erase(work.device.id);xSemaphoreGive(fleetMutex);return {result.status==HTTPC_ERROR_READ_TIMEOUT?"Timeout":"Offline",result.error+"; movement was not retried"};}
 sender.epoch=response["epoch"]|sender.epoch;xSemaphoreTake(fleetMutex,portMAX_DELAY);if(std::string(response["reason"]|"")=="Peer session expired or wrong boot")senderSessions.erase(work.device.id);else senderSessions[work.device.id]=sender;xSemaphoreGive(fleetMutex);
 return {response["accepted"]|false?"Accepted":response["state"]|"Rejected",response["reason"]|""};
}
void fleetWorkerTask(void*){for(;;){xSemaphoreTake(fleetMutex,portMAX_DELAY);auto work=fleetDispatch.nextWork(millis());xSemaphoreGive(fleetMutex);if(work){auto result=doPeerWork(*work);xSemaphoreTake(fleetMutex,portMAX_DELAY);fleetDispatch.complete(work->id,result,millis());if(result.state=="Offline"||result.state=="Timeout"){auto& s=fleetStatuses[work->device.id];s.state=result.state;}xSemaphoreGive(fleetMutex);}vTaskDelay(pdMS_TO_TICKS(20));}}
void queuePeerPolls(){static uint32_t last=0;if(uint32_t(millis()-last)<3000)return;last=millis();xSemaphoreTake(fleetMutex,portMAX_DELAY);if(fleetRegistry->config.dashboard)for(auto d:fleetRegistry->config.devices)if(d.id!=deviceId)fleetDispatch.poll(d);xSemaphoreGive(fleetMutex);}
