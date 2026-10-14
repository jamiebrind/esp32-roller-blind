#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <esp_wifi.h>
#include <esp_sntp.h>
#include <Preferences.h>
#include <ArduinoJson.h>
#include "core/Motion.h"
#include "core/CommandGate.h"
#include "core/Quadrature.h"
#include "core/Scheduler.h"
#include "core/SettingsJson.h"
#include "Drv8871.h"
#include "DeviceStorage.h"
#include "WebAssets.h"
#include "AdminStorage.h"
#include "core/AdminAuth.h"
#include "OtaAdapter.h"
#include "FleetGlobals.h"
#include "WifiRetry.h"
constexpr const char* FIRMWARE_VERSION="2.0.2";

constexpr uint8_t IN1=0,IN2=1,ENC_A=3,ENC_B=4;
WebServer web(80);Motion motion;Settings settings;ClockState clockState;Scheduler scheduler;DeviceStorage storage;
SemaphoreHandle_t stateMutex;portMUX_TYPE encoderMutex=portMUX_INITIALIZER_UNLOCKED;
portMUX_TYPE syncMutex=portMUX_INITIALIZER_UNLOCKED;
volatile int64_t encoderCount=0;Quadrature quadrature;volatile bool newTime=false;int64_t syncUtc=0;
CommandGate commands;String serialLine;std::string jogOwner;Preferences wifiPreferences;
std::string wifiName,wifiKey;bool saveWifi=false,wasConnected=false;WifiRetry wifiRetry;
int64_t previousUtc=0;bool ready=false,ntpRestart=false;
int appliedDirection=0;bool maintenance=false;
std::string deviceId,localName,localHostname;
AdminStorage adminStorage;AdminAuth adminAuth([](const std::string& p){return adminStorage.verify(p);},[]{return randomHex();});
OtaAdapter otaPort;UpdateCoordinator updater(otaPort);
bool requireAdmin();bool requestOrigin();

int64_t readEncoder(){portENTER_CRITICAL(&encoderMutex);int64_t value=encoderCount;portEXIT_CRITICAL(&encoderMutex);return value;}
void encoderISR(){
 uint8_t ab=(digitalRead(ENC_A)?2:0)|(digitalRead(ENC_B)?1:0);
 portENTER_CRITICAL_ISR(&encoderMutex);encoderCount+=quadrature.update(ab);portEXIT_CRITICAL_ISR(&encoderMutex);
}
void timeSynced(struct timeval* value){portENTER_CRITICAL(&syncMutex);syncUtc=value->tv_sec;newTime=true;portEXIT_CRITICAL(&syncMutex);}
void configureNtp(){configTzTime("GMT0BST,M3.5.0/1,M10.5.0",settings.ntp1.c_str(),settings.ntp2.c_str());}

void motorTask(void*){
 DriverOutput driver;int applied=0;
 auto write=[](int a,int b){ledcWrite(IN1,a);ledcWrite(IN2,b);};
 for(;;){
  xSemaphoreTake(stateMutex,portMAX_DELAY);uint32_t now=millis();motion.tick(now,readEncoder());auto s=motion.status();if(maintenance){motion.stop(now);s=motion.status();}int desired=settings.reverseMotor?-s.direction:s.direction;
  if(!applied&&desired){
   if(!storage.markMoving({s.count,s.positionKnown&&!motion.inCalibration(),true,settings.calibrationVersion})){motion.latchFault("Cannot save motion marker",now);desired=0;}
  }
  driver.apply(desired,255,write);
  if(applied&&!desired){auto stopped=motion.status();if(!storage.savePosition({stopped.count,stopped.positionKnown&&!motion.inCalibration(),false,settings.calibrationVersion}))motion.latchFault("Cannot save stopped position",now);}
  if(!s.direction)jogOwner.clear();applied=desired;appliedDirection=desired;xSemaphoreGive(stateMutex);vTaskDelay(pdMS_TO_TICKS(5));
 }
}

void replyError(const std::string& error,int status=400){JsonDocument d;d["ok"]=false;d["error"]=error;String text;serializeJson(d,text);web.send(status,"application/json",text);}
void replyOK(){xSemaphoreTake(stateMutex,portMAX_DELAY);uint32_t epoch=commands.epoch();xSemaphoreGive(stateMutex);JsonDocument d;d["ok"]=true;d["epoch"]=epoch;String text;serializeJson(d,text);web.send(200,"application/json",text);}
#include "AdminRoutes.h"
bool body(JsonDocument& d,bool command=true){
 if(!requireAdmin())return false;
 if(web.header("X-Blind-Request")!="1"){replyError("Open the local control webpage",403);return false;}
 String origin=web.header("Origin");if(!origin.isEmpty()&&origin!="http://"+web.hostHeader()){replyError("Request origin rejected",403);return false;}
 String raw=web.arg("plain");if(raw.length()>8192||deserializeJson(d,raw)||!d.is<JsonObject>()){replyError("Invalid JSON request");return false;}
 if(command){
  if(!d["session"].is<const char*>()||!d["sequence"].is<uint32_t>()){replyError("Invalid command session",409);return false;}
  xSemaphoreTake(stateMutex,portMAX_DELAY);bool accepted=commands.accept(d["session"].as<std::string>(),d["sequence"].as<uint32_t>());xSemaphoreGive(stateMutex);
  if(!accepted){replyError("Old command ignored; reload the webpage",409);return false;}
 }return true;
}
#include "UpdateRoutes.h"
#include "PeerRoutes.h"
#include "FleetWorker.h"
#include "FleetRoutes.h"
void sendSettings(){if(!requireAdmin())return;JsonDocument d;settingsToJson(settings,d);String text;serializeJson(d,text);web.sendHeader("Cache-Control","no-store");web.send(200,"application/json",text);}
void sendStatus(){if(!requireAdmin())return;
 xSemaphoreTake(stateMutex,portMAX_DELAY);auto s=motion.status();uint32_t now=millis();bool valid=clockState.valid(now);int64_t utc=valid?clockState.utcNow(now):0;
 JsonDocument d;d["deviceId"]=deviceId;d["name"]=localName;d["version"]=FIRMWARE_VERSION;d["updating"]=maintenance;d["epoch"]=commands.epoch();d["locationSet"]=settings.locationSet;d["direction"]=s.direction;d["rawCount"]=readEncoder();d["count"]=s.count;d["positionPercent"]=motion.percent();d["positionKnown"]=s.positionKnown;d["calibrated"]=s.calibrated;d["calibrating"]=motion.inCalibration();d["atUpper"]=motion.atUpper();d["atLower"]=motion.atLower();d["fault"]=s.fault;
 d["wifi"]=WiFi.status()==WL_CONNECTED;d["timeValid"]=valid;d["utc"]=utc;d["syncAge"]=clockState.synced?clockState.lastSyncAgeMs(now)/1000:0;d["bst"]=valid&&UkTime::isBst(utc);d["preview"]=false;
 Settings config=settings;xSemaphoreGive(stateMutex);
 if(valid&&config.locationSet){auto sun=Solar::events(UkTime::fromUtc(utc).date,config.latitude,config.longitude);if(sun.sunriseUtc)d["sunriseUtc"]=*sun.sunriseUtc;if(sun.sunsetUtc)d["sunsetUtc"]=*sun.sunsetUtc;}
 if(valid){auto next=scheduler.nextEvent(config,utc);if(next){auto n=d["next"].to<JsonObject>();n["utc"]=next->utc;n["targetPercent"]=next->targetPercent;n["ruleId"]=next->ruleId;}}
 String text;serializeJson(d,text);web.sendHeader("Cache-Control","no-store");web.send(200,"application/json",text);
}

void setupWeb(){
 const char* headers[]={"Origin","X-Blind-Request","X-Blind-Auth","X-Peer-Key"};web.collectHeaders(headers,4);registerAdminRoutes();registerUpdateRoutes();registerPeerRoutes();registerFleetRoutes();
 web.on("/",HTTP_GET,[]{web.sendHeader("Cache-Control","no-store");web.send_P(200,"text/html",INDEX_HTML);});
 web.on("/app.css",HTTP_GET,[]{web.send_P(200,"text/css",APP_CSS);});
 web.on("/app.js",HTTP_GET,[]{web.send_P(200,"text/javascript",APP_JS);});
 web.on("/fleet.mjs",HTTP_GET,[]{web.send_P(200,"text/javascript",FLEET_JS);});
 web.on("/firmware.mjs",HTTP_GET,[]{web.send_P(200,"text/javascript",FIRMWARE_JS);});
 web.on("/control.mjs",HTTP_GET,[]{web.send_P(200,"text/javascript",CONTROL_JS);});
 web.on("/api/session",HTTP_POST,[]{JsonDocument request;if(!body(request,false))return;String token=String(esp_random(),HEX)+String(esp_random(),HEX);commands.issue(token.c_str());JsonDocument d;d["ok"]=true;d["session"]=token;d["epoch"]=commands.epoch();String text;serializeJson(d,text);web.send(200,"application/json",text);});
 web.on("/api/status",HTTP_GET,sendStatus);
 web.on("/api/settings",HTTP_GET,sendSettings);
 web.on("/api/settings",HTTP_POST,[]{
  JsonDocument request;if(!body(request))return;xSemaphoreTake(stateMutex,portMAX_DELAY);
  if(maintenance||motion.status().direction||appliedDirection){xSemaphoreGive(stateMutex);replyError("Stop the blind before changing settings",409);return;}
  Settings candidate=settings;auto error=settingsFromJson(request.as<JsonVariantConst>(),candidate,false);
  if(!error.empty()){xSemaphoreGive(stateMutex);replyError(error);return;}
  bool reverseChanged=candidate.reverseMotor!=settings.reverseMotor;
  if(reverseChanged){candidate.span=0;++candidate.calibrationVersion;if(!storage.savePosition({0,false,false})){xSemaphoreGive(stateMutex);replyError("Cannot save calibration reset",500);return;}}
  if(!storage.saveSettings(candidate)){xSemaphoreGive(stateMutex);replyError("Settings save failed; previous settings retained",500);return;}
  settings=candidate;motion.maxRuntime=settings.maxRuntime;motion.stallMs=settings.stallMs;motion.minProgressCounts=settings.minProgressCounts;if(reverseChanged)motion.invalidateCalibration();
  previousUtc=clockState.valid(millis())?clockState.utcNow(millis()):0;ntpRestart=true;
  JsonDocument d;d["ok"]=true;JsonDocument publicSettings;settingsToJson(settings,publicSettings);d["settings"]=publicSettings;String text;serializeJson(d,text);
  xSemaphoreGive(stateMutex);web.send(200,"application/json",text);
 });
 web.on("/api/move",HTTP_POST,[]{
  JsonDocument r;if(!body(r,false))return;std::string action=r["action"]|"";bool accepted=false;
  // Validate Stop epoch and dispatch under one lock: a fleet Stop cannot interleave.
  xSemaphoreTake(stateMutex,portMAX_DELAY);uint32_t now=millis();
  if(action!="stop"){
   if(!r["session"].is<const char*>()||!r["sequence"].is<uint32_t>()||!commands.accept(r["session"].as<std::string>(),r["sequence"].as<uint32_t>())||!r["epoch"].is<uint32_t>()||!commands.canMove(r["epoch"].as<uint32_t>())){xSemaphoreGive(stateMutex);replyError("Stale movement ignored",409);return;}
  }
  if(action!="stop"&&maintenance){xSemaphoreGive(stateMutex);replyError("Firmware update in progress",409);return;}
  if(action=="stop"){commands.stop();if(peerProtocol)peerProtocol->stop();jogOwner.clear();motion.stop(now);accepted=true;}
  else if(action=="jog"&&r["direction"].is<int>()){
   std::string owner=r["session"].as<std::string>();
   if(r["begin"].is<bool>()&&r["begin"].as<bool>()){accepted=motion.newJog(r["direction"],now);if(accepted)jogOwner=owner;}
   else if(owner==jogOwner&&motion.status().direction)accepted=motion.jog(r["direction"],now);
  }
  else if(action=="raise"||action=="lower")accepted=motion.moveTo(action=="raise"?100:0,now);
  else if(action=="target"&&r["targetPercent"].is<int>())accepted=motion.moveTo(r["targetPercent"],now);
  xSemaphoreGive(stateMutex);if(accepted)replyOK();else replyError("Movement rejected: already at target, or check calibration, position or fault",409);
 });
 web.on("/api/calibration",HTTP_POST,[]{
  JsonDocument r;if(!body(r))return;std::string action=r["action"]|"";bool ok=false;std::string error;
  xSemaphoreTake(stateMutex,portMAX_DELAY);uint32_t now=millis();auto s=motion.status();int64_t raw=readEncoder();
  if(maintenance||s.direction||appliedDirection){error="Release the jog control and wait for Stop before saving";}
  else if(action=="begin"){
   Settings candidate=settings;candidate.span=0;++candidate.calibrationVersion;
   if(storage.savePosition({s.count,false,false,candidate.calibrationVersion})&&storage.saveSettings(candidate)){settings=candidate;motion.beginCalibration();ok=true;}else error="Cannot save calibration state";
  }
  else if(action=="saveUpper")ok=motion.saveUpper(raw);
  else if(action=="saveLower")ok=motion.saveLower(raw);
  else if(action=="finish"){
   if(motion.inCalibration()&&s.calibrated&&s.fault.empty()){Settings candidate=settings;candidate.span=motion.span();candidate.encoderSign=motion.encoderSign();++candidate.calibrationVersion;
    if(storage.saveSettings(candidate)&&storage.savePosition({s.count,true,false,candidate.calibrationVersion})){settings=candidate;ok=motion.endCalibration();}else error="Cannot save calibration";
   }
  }else if(action=="reset"){
   Settings candidate=settings;candidate.span=0;++candidate.calibrationVersion;if(storage.savePosition({0,false,false})&&storage.saveSettings(candidate)){settings=candidate;motion.invalidateCalibration();ok=true;}else error="Cannot save reset";
  }else if(action=="referenceUpper"){
   if(s.calibrated){if(storage.savePosition({0,true,false,settings.calibrationVersion})){motion.referenceUpper(raw);ok=true;}else error="Cannot save reference";}
  }
  xSemaphoreGive(stateMutex);if(ok)replyOK();else replyError(error.empty()?"Complete calibration in order, with motor stopped":error,409);
 });
 web.on("/api/fault/reset",HTTP_POST,[]{JsonDocument r;if(!body(r))return;xSemaphoreTake(stateMutex,portMAX_DELAY);if(!maintenance)motion.resetFault();xSemaphoreGive(stateMutex);replyOK();});
 web.onNotFound([]{replyError("Not found",404);});web.begin();
}

void usbSetup(){
 while(Serial.available()){
  char c=Serial.read();if(c=='\r')continue;
  if(c!='\n'){if(serialLine.length()<160)serialLine+=c;else serialLine="INVALID";continue;}
  if(serialLine=="STATUS")Serial.printf("Scheduled blind: Wi-Fi %s; http://%s/\n",WiFi.status()==WL_CONNECTED?"connected":"offline",WiFi.localIP().toString().c_str());
  else if(serialLine.startsWith("ADMIN\t")){
   xSemaphoreTake(stateMutex,portMAX_DELAY);bool allowed=!maintenance&&!motion.status().direction&&!appliedDirection;xSemaphoreGive(stateMutex);
   if(allowed&&adminStorage.provision(serialLine.substring(6).c_str()))Serial.println("Administrator password saved; reconnect web sessions");else Serial.println("Password setup rejected; stop movement, use 8-63 characters");
  }
  else if(serialLine.startsWith("WIFI\t")){
   xSemaphoreTake(stateMutex,portMAX_DELAY);bool moving=maintenance||motion.status().direction!=0||appliedDirection!=0;xSemaphoreGive(stateMutex);
   int split=serialLine.indexOf('\t',5);String name=serialLine.substring(5,split),key=serialLine.substring(split+1);
   if(moving)Serial.println("Stop movement before changing Wi-Fi");
   else if(split>5&&name.length()<=32&&key.length()>=8&&key.length()<=63&&key.indexOf('\t')<0){wifiName=name.c_str();wifiKey=key.c_str();WiFi.disconnect(false,false);WiFi.begin(wifiName.c_str(),wifiKey.c_str());wifiRetry.attempted(millis());saveWifi=true;Serial.println("Connecting; password not echoed");}
   else Serial.println("Invalid Wi-Fi setup");
  }
  serialLine="";
 }
}

void setup(){
 pinMode(IN1,OUTPUT);pinMode(IN2,OUTPUT);digitalWrite(IN1,LOW);digitalWrite(IN2,LOW);Serial.begin(115200);
 stateMutex=xSemaphoreCreateMutex();fleetMutex=xSemaphoreCreateMutex();if(!stateMutex||!fleetMutex){Serial.println("State mutex failed; motor disabled");return;}
 if(!storage.begin()||!adminStorage.begin()||!fleetStorage.begin()){Serial.println("Storage failed; motor disabled");return;}
 if(!storage.migrateManualLimits()){Serial.println("Limit migration failed; motor disabled");return;}
 settings=storage.loadSettings();motion.restore(storage.loadPosition(),settings.span,settings.encoderSign,settings.calibrationVersion);motion.invalidatePosition();motion.maxRuntime=settings.maxRuntime;motion.stallMs=settings.stallMs;motion.minProgressCounts=settings.minProgressCounts;scheduler.restoreClaims(storage.loadClaims());
 pinMode(ENC_A,INPUT_PULLUP);pinMode(ENC_B,INPUT_PULLUP);quadrature.reset((digitalRead(ENC_A)?2:0)|(digitalRead(ENC_B)?1:0));
 attachInterrupt(digitalPinToInterrupt(ENC_A),encoderISR,CHANGE);attachInterrupt(digitalPinToInterrupt(ENC_B),encoderISR,CHANGE);
 if(!ledcAttach(IN1,1000,8)||!ledcAttach(IN2,1000,8)){Serial.println("PWM setup failed; motor disabled");return;}
 ledcWrite(IN1,0);ledcWrite(IN2,0);
 if(xTaskCreate(motorTask,"encoder-motor",6144,nullptr,3,nullptr)!=pdPASS){Serial.println("Motor task failed");return;}
 deviceId=WiFi.macAddress().c_str();char idbuf[17];snprintf(idbuf,sizeof(idbuf),"%012llx",(unsigned long long)ESP.getEfuseMac());deviceId=idbuf;localHostname="blind-"+deviceId;localName="Blind "+deviceId.substr(6);
 fleetRegistry=std::make_unique<FleetRegistry>(Device{deviceId,localName,localHostname+".local",""});fleetRegistry->config=fleetStorage.load({deviceId,localName,localHostname+".local",""});localName=fleetRegistry->config.name;peerProtocol=std::make_unique<PeerProtocol>(randomHex());
 if(xTaskCreate(fleetWorkerTask,"fleet-net-1",12288,nullptr,1,nullptr)!=pdPASS||xTaskCreate(fleetWorkerTask,"fleet-net-2",12288,nullptr,1,nullptr)!=pdPASS){Serial.println("Fleet worker setup failed");return;}
 WiFi.setHostname(localHostname.c_str());WiFi.mode(WIFI_STA);WiFi.setAutoReconnect(false);WiFi.disconnect(false,false);delay(200);esp_wifi_set_country_code("GB",false);WiFi.setSleep(false);
 wifiPreferences.begin("blind-wifi",false);if(wifiPreferences.getBool("lowtx",false))WiFi.setTxPower(WIFI_POWER_8_5dBm);WiFi.setAutoReconnect(true);
 wifiName=wifiPreferences.getString("ssid","").c_str();wifiKey=wifiPreferences.getString("password","").c_str();
 sntp_set_time_sync_notification_cb(timeSynced);setupWeb();configureNtp();if(!wifiName.empty()){wifiRetry.attempted(millis());WiFi.begin(wifiName.c_str(),wifiKey.c_str());}
 ready=true;Serial.println("Scheduled blind ready: DRV8871/encoder; motor stays disabled until commanded");
}

void loop(){
 if(!ready){delay(100);return;}usbSetup();web.handleClient();serviceUpdate();queuePeerPolls();adminAuth.tick(millis());uint32_t now=millis();
 portENTER_CRITICAL(&syncMutex);bool update=newTime;int64_t timestamp=syncUtc;newTime=false;portEXIT_CRITICAL(&syncMutex);
 xSemaphoreTake(stateMutex,portMAX_DELAY);
 if(update){bool wasValid=clockState.valid(now);int64_t old=clockState.utcNow(now);clockState.sync(timestamp,now);if(!wasValid||std::llabs(timestamp-old)>30)previousUtc=timestamp;}
 bool valid=clockState.valid(now);int64_t utc=valid?clockState.utcNow(now):0;auto s=motion.status();Settings config=settings;
 bool busy=maintenance||s.direction||appliedDirection||!s.fault.empty()||motion.inCalibration();xSemaphoreGive(stateMutex);
 if(valid&&previousUtc&&!maintenance){auto due=scheduler.crossedEvents(config,previousUtc,utc,true);
  if(!due.empty()){
   xSemaphoreTake(stateMutex,portMAX_DELAY);auto current=motion.status();
   bool saved=true;
   for(auto event:due){if(!storage.claimEvent(event,scheduler,settings)){saved=false;break;}scheduler.claim(event);}
   if(!saved)motion.latchFault("Cannot save scheduled event",millis());
   else if(!busy&&!current.direction&&!appliedDirection&&current.positionKnown&&current.fault.empty()&&!motion.inCalibration())motion.moveTo(due.front().targetPercent,millis());xSemaphoreGive(stateMutex);
  }
 }
 previousUtc=utc;
 if(ntpRestart){configureNtp();ntpRestart=false;}
 bool connected=WiFi.status()==WL_CONNECTED;
 if(connected&&!wasConnected){
  if(saveWifi){wifiPreferences.putString("ssid",wifiName.c_str());wifiPreferences.putString("password",wifiKey.c_str());wifiKey.clear();saveWifi=false;}
  MDNS.begin(localHostname.c_str());MDNS.addService("http","tcp",80);Serial.printf("Webpage http://%s/ or http://%s.local/\n",WiFi.localIP().toString().c_str(),localHostname.c_str());
 }
 if(!wifiName.empty()&&wifiRetry.due(connected,millis())){wifiRetry.attempted(millis());WiFi.reconnect();}
 wasConnected=connected;delay(2);
}
