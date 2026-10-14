#pragma once
#include <string>
#include <vector>
#include <optional>
#include <cstdint>
#include <cmath>
#include <set>
enum class EventKind{Fixed,Sunrise,Sunset};
struct Rule{uint32_t id;bool enabled;uint8_t weekdays;EventKind kind;int localMinute,offsetMinutes,targetPercent;};
struct Settings{
 uint32_t schemaVersion=1,sequence=0,calibrationVersion=0,minProgressCounts=4;double latitude=0,longitude=0;bool locationSet=false,schedulesEnabled=false,reverseMotor=false;
 std::vector<Rule> rules;int64_t span=0;int encoderSign=1;uint32_t maxRuntime=60000,stallMs=1000;
 std::string ntp1="pool.ntp.org",ntp2="time.cloudflare.com";
};
struct SettingsValidator{
 static std::string validate(const Settings& s){
  if(s.schemaVersion!=1)return "Unsupported settings version";
  if(s.locationSet&&(!std::isfinite(s.latitude)||!std::isfinite(s.longitude)||s.latitude< -90||s.latitude>90||s.longitude< -180||s.longitude>180))return "Enter valid latitude and longitude";
  if(s.rules.size()>8)return "Maximum eight schedule rules";
  if(s.span<0||s.span>1000000000LL||(s.encoderSign!=1&&s.encoderSign!=-1))return "Invalid travel calibration";
  if(s.minProgressCounts<1||s.minProgressCounts>100000)return "Invalid minimum encoder progress";
  if(s.maxRuntime<1000||s.maxRuntime>60000||s.stallMs<100||s.stallMs>5000)return "Invalid motor timeouts";
  if(s.ntp1.empty()||s.ntp1.size()>100||s.ntp2.empty()||s.ntp2.size()>100)return "Enter NTP server names";
  std::set<uint32_t> ids;
  for(auto r:s.rules){
   if(!r.id||!ids.insert(r.id).second)return "Schedule IDs must be unique";
   if(!r.weekdays||r.weekdays>127)return "Select at least one weekday";
   if(r.localMinute<0||r.localMinute>1439||r.offsetMinutes< -720||r.offsetMinutes>720||r.targetPercent<0||r.targetPercent>100)return "Invalid schedule time, offset or position";
   if(r.kind!=EventKind::Fixed&&r.kind!=EventKind::Sunrise&&r.kind!=EventKind::Sunset)return "Invalid schedule type";
  }return {};
 }
};
struct StorageRecord{uint32_t version,sequence;std::string payload;bool checksumValid;};
inline std::optional<StorageRecord> selectRecord(StorageRecord a,StorageRecord b){
 bool av=a.version==1&&a.checksumValid,bv=b.version==1&&b.checksumValid;
 if(!av&&!bv)return {};if(!av)return b;if(!bv)return a;
 return int32_t(b.sequence-a.sequence)>0?b:a;
}
inline uint32_t checksum(const std::string& value){uint32_t h=2166136261U;for(unsigned char c:value){h^=c;h*=16777619U;}return h;}
