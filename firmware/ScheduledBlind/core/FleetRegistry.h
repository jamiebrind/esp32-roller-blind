#pragma once
#include <string>
#include <vector>
#include <set>
#include <algorithm>
#include <cctype>
#include <cstdint>
struct Device {std::string id,name,endpoint,pairingKey;uint32_t epoch=0;std::string expectedBoot;};struct Group {std::string id,name;std::vector<std::string> members;};struct FleetConfig {bool dashboard=false;std::string name;std::vector<Device> devices;std::vector<Group> groups;};
class FleetRegistry {
 std::string localId;
 static bool label(const std::string& s){return !s.empty()&&s.size()<=32&&std::none_of(s.begin(),s.end(),[](char c){return(uint8_t)c<32;});}
public:
 FleetConfig config;explicit FleetRegistry(Device local):localId(local.id){config.name=local.name;config.devices.push_back(local);}
 static bool endpointValid(std::string e){
  if(e.empty()||e.size()>100||e.find_first_of("/@?#\\ ")!=std::string::npos||std::any_of(e.begin(),e.end(),[](char c){return (uint8_t)c<33;}))return false;
  auto colon=e.find(':');if(colon!=std::string::npos){auto port=e.substr(colon+1);if(port.empty()||port.size()>5||!std::all_of(port.begin(),port.end(),[](char c){return isdigit((unsigned char)c);}))return false;unsigned n=std::stoul(port);if(!n||n>65535)return false;e=e.substr(0,colon);}
  if(e.empty()||e.front()=='.'||e.back()=='.')return false;
  if(std::all_of(e.begin(),e.end(),[](char c){return isdigit((unsigned char)c)||c=='.';})){
   std::vector<int> a;size_t start=0;while(start<e.size()){auto end=e.find('.',start);if(end==std::string::npos)end=e.size();auto part=e.substr(start,end-start);if(part.empty()||part.size()>3)return false;int n=std::stoi(part);if(n>255)return false;a.push_back(n);start=end+1;}
   return a.size()==4&&a[0]!=0&&a[0]!=127&&a[0]<224&&a[3]!=0&&a[3]!=255;
  }
  return std::all_of(e.begin(),e.end(),[](char c){return isalnum((unsigned char)c)||c=='.'||c=='-';})&&e!="localhost";
 }
 std::string add(Device d){if(config.devices.size()>=8)return "Maximum eight blinds";if(!label(d.id)||!label(d.name)||!endpointValid(d.endpoint)||d.pairingKey.empty()||d.pairingKey.size()>64)return "Invalid device fields";for(auto x:config.devices)if(x.id==d.id||x.endpoint==d.endpoint)return "Duplicate identity or endpoint";config.devices.push_back(d);return {};}
 std::string edit(Device d){for(auto& x:config.devices)if(x.id==d.id){if(!label(d.name)||(d.id!=localId&&!endpointValid(d.endpoint)))return "Invalid device fields";for(auto other:config.devices)if(other.id!=d.id&&other.endpoint==d.endpoint)return "Duplicate endpoint";if(d.id==localId){x.name=d.name;config.name=d.name;}else x=d;return {};}return "Unknown blind";}
 std::string remove(std::string id){if(id==localId)return "Cannot remove this blind";auto old=config.devices.size();config.devices.erase(std::remove_if(config.devices.begin(),config.devices.end(),[&](auto d){return d.id==id;}),config.devices.end());if(old==config.devices.size())return "Unknown blind";for(auto& g:config.groups)g.members.erase(std::remove(g.members.begin(),g.members.end(),id),g.members.end());config.groups.erase(std::remove_if(config.groups.begin(),config.groups.end(),[](auto g){return g.members.empty();}),config.groups.end());return {};}
 std::string saveGroup(Group g){if(!label(g.id)||!label(g.name)||g.members.empty()||g.members.size()>8)return "Invalid group";std::set<std::string> ids;for(auto id:g.members){if(!ids.insert(id).second||std::none_of(config.devices.begin(),config.devices.end(),[&](auto d){return d.id==id;}))return "Unknown or duplicate group member";}for(auto& old:config.groups)if(old.id==g.id){old=g;return {};}if(config.groups.size()>=8)return "Maximum eight groups";config.groups.push_back(g);return {};}
 std::vector<Device> snapshot(std::string id)const{std::vector<Device> result;for(auto g:config.groups)if(g.id==id){for(auto member:g.members)for(auto d:config.devices)if(d.id==member)result.push_back(d);return result;}for(auto d:config.devices)if(d.id==id)return {d};return {};}
};
