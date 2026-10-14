#pragma once
#include <map>
#include <string>
#include <cstdint>
class CommandGate {
 std::map<std::string,uint32_t> sequences;uint32_t generation=1;
public:
 void issue(const std::string& token){if(sequences.size()>=16)sequences.erase(sequences.begin());sequences[token]=0;}
 bool accept(const std::string& token,uint32_t sequence){auto i=sequences.find(token);if(i==sequences.end()||!sequence||sequence<=i->second)return false;i->second=sequence;return true;}
 bool acceptMovement(const std::string& token,uint32_t sequence,uint32_t epoch){return canMove(epoch)&&accept(token,sequence);}
 uint32_t epoch()const{return generation;}
 void stop(){++generation;}
 bool canMove(uint32_t e)const{return e==generation;}
};
