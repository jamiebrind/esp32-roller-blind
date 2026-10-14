#pragma once
std::string updateId,updateOwner;bool uploadAllowed=false;
void firmwareInfo(){if(!requireAdmin())return;JsonDocument d;d["ok"]=true;d["hardware"]="esp32c3-drv8871";d["version"]=FIRMWARE_VERSION;d["deviceId"]=deviceId;d["name"]=localName;d["maximumBytes"]=1966080;d["settingsSchema"]=1;d["received"]=updater.received();d["updating"]=updater.blocksMovement();d["state"]=int(updater.state());d["error"]=updater.error();
#if defined(CONFIG_BOOTLOADER_APP_ROLLBACK_ENABLE) && CONFIG_BOOTLOADER_APP_ROLLBACK_ENABLE
 d["bootloaderRollbackEnabled"]=true;
#else
 d["bootloaderRollbackEnabled"]=false;
#endif
 // Arduino confirms OTA before setup by default; no application health rollback is promised.
 d["automaticRollback"]=false;d["applicationHealthChecks"]=false;
 String text;serializeJson(d,text);web.sendHeader("Cache-Control","no-store");web.send(200,"application/json",text);}
void registerUpdateRoutes(){
 web.on("/api/firmware/info",HTTP_GET,firmwareInfo);
 web.on("/api/firmware/prepare",HTTP_POST,[]{JsonDocument d;if(!body(d,false))return;
  if(!d["hardware"].is<const char*>()||!d["version"].is<const char*>()||!d["settingsSchema"].is<uint32_t>()||!d["length"].is<uint32_t>()||!d["sha256"].is<const char*>()){replyError("Invalid firmware manifest");return;}
  FirmwareManifest m{d["hardware"].as<std::string>(),d["version"].as<std::string>(),d["settingsSchema"].as<uint32_t>(),d["length"].as<uint32_t>(),d["sha256"].as<std::string>()};
  xSemaphoreTake(stateMutex,portMAX_DELAY);bool calibrating=motion.inCalibration();xSemaphoreGive(stateMutex);if(calibrating){replyError("Finish calibration first",409);return;}
  std::string candidate=randomHex();if(!updater.request(m,candidate,millis())){replyError(updater.blocksMovement()?"An update is already active":updater.error(),409);return;}
  updateId=candidate;updateOwner=web.header("X-Blind-Auth").c_str();uploadAllowed=false;
  xSemaphoreTake(stateMutex,portMAX_DELAY);maintenance=true;commands.stop();if(peerProtocol)peerProtocol->stop();jogOwner.clear();motion.stop(millis());xSemaphoreGive(stateMutex);
  xSemaphoreTake(fleetMutex,portMAX_DELAY);fleetDispatch.cancelPending();xSemaphoreGive(fleetMutex);
  JsonDocument out;out["ok"]=true;out["uploadId"]=updateId;out["epoch"]=commands.epoch();String text;serializeJson(out,text);web.send(200,"application/json",text);
 });
 web.on("/api/firmware/cancel",HTTP_POST,[]{JsonDocument d;if(!body(d,false))return;if(updater.state()==UpdateState::RebootPending){replyError("Reboot already pending",409);return;}updater.abort("Cancelled by owner");replyOK();});
 web.on("/api/firmware/upload",HTTP_POST,[]{
  if(!uploadAllowed){replyError("Upload session rejected",403);return;}
  if(updater.state()!=UpdateState::RebootPending){replyError(updater.error().empty()?"Upload incomplete":updater.error(),400);return;}
  replyOK();
 },[]{
  auto& upload=web.upload();if(upload.status==UPLOAD_FILE_START){
   uploadAllowed=requestOrigin()&&secureEqual(web.header("X-Blind-Auth").c_str(),updateOwner)&&secureEqual(web.arg("id").c_str(),updateId)&&updater.state()==UpdateState::Receiving;
  }else if(upload.status==UPLOAD_FILE_WRITE){if(uploadAllowed&&!updater.chunk(updateId,upload.buf,upload.currentSize,millis()))uploadAllowed=false;}
  else if(upload.status==UPLOAD_FILE_END){if(uploadAllowed&&!updater.finish(updateId,millis()))uploadAllowed=false;}
  else if(upload.status==UPLOAD_FILE_ABORTED){if(uploadAllowed)updater.abort("Browser disconnected");uploadAllowed=false;}
 });
}
void serviceUpdate(){
 updater.tick(millis());
 if(updater.state()==UpdateState::WaitingForStop){
  xSemaphoreTake(stateMutex,portMAX_DELAY);auto s=motion.status();bool stopped=appliedDirection==0&&s.direction==0;bool cal=motion.inCalibration();PositionRecord pos{s.count,s.positionKnown&&!cal,false,settings.calibrationVersion};xSemaphoreGive(stateMutex);
  if(stopped){bool saved=storage.savePosition(pos);updater.stopped(0,saved,cal,millis());}
 }
 bool blocked=updater.blocksMovement();xSemaphoreTake(stateMutex,portMAX_DELAY);maintenance=blocked;if(blocked){motion.stop(millis());previousUtc=clockState.valid(millis())?clockState.utcNow(millis()):0;}xSemaphoreGive(stateMutex);
 if(updater.rebootDue(millis()))ESP.restart();
}
