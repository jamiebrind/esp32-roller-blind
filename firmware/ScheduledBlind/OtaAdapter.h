#pragma once
#include <Update.h>
#include <esp_ota_ops.h>
#include <esp_app_format.h>
#include <mbedtls/sha256.h>
#include <vector>
#include "core/UpdateCoordinator.h"
#include "AdminCrypto.h"
class OtaAdapter:public OtaPort {
 mbedtls_sha256_context sha;bool hashing=false,opened=false,headerValid=false;std::vector<uint8_t> prefix;size_t total=0;std::string expected;
 bool checkedHeader(){return prefix.size()>=288&&prefix[0]==0xe9&&prefix[12]==5&&prefix[13]==0&&prefix[32]==0x32&&prefix[33]==0x54&&prefix[34]==0xcd&&prefix[35]==0xab;}
public:
 OtaAdapter(){mbedtls_sha256_init(&sha);}~OtaAdapter(){mbedtls_sha256_free(&sha);}
 bool begin(const FirmwareManifest& m)override{auto part=esp_ota_get_next_update_partition(nullptr);if(!part||m.length>part->size)return false;prefix.clear();total=0;headerValid=false;expected=m.sha256;mbedtls_sha256_starts(&sha,0);hashing=true;opened=Update.begin(m.length,U_FLASH);return opened;}
 size_t write(const uint8_t* bytes,size_t n)override{
  if(!opened||!hashing)return 0;mbedtls_sha256_update(&sha,bytes,n);total+=n;
  size_t used=0;if(!headerValid){size_t take=std::min(n,size_t(288-prefix.size()));prefix.insert(prefix.end(),bytes,bytes+take);used=take;if(prefix.size()<288)return n;if(!checkedHeader())return 0;headerValid=true;if(Update.write(prefix.data(),prefix.size())!=prefix.size())return 0;prefix.clear();}
  return used+(n>used?Update.write((uint8_t*)bytes+used,n-used):0);
 }
 bool verify(const FirmwareManifest& m)override{if(!opened||!headerValid||!hashing||total!=m.length)return false;uint8_t digest[32];mbedtls_sha256_finish(&sha,digest);hashing=false;return secureEqual(hexBytes(digest,32),expected);}
 bool selectCandidate()override{bool ok=opened&&Update.end(false);opened=false;return ok;}
 void abort()override{if(opened)Update.abort();opened=hashing=false;prefix.clear();}
};
