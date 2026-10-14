#pragma once
#include <Arduino.h>
#include <esp_system.h>
#include <mbedtls/pkcs5.h>
#include <mbedtls/md.h>
#include <mbedtls/sha256.h>
inline std::string hexBytes(const uint8_t* b,size_t n){const char* digits="0123456789abcdef";std::string s;s.reserve(n*2);for(size_t i=0;i<n;i++){s+=digits[b[i]>>4];s+=digits[b[i]&15];}return s;}
inline std::string randomHex(size_t n=32){uint8_t bytes[32];if(n>32)n=32;esp_fill_random(bytes,n);return hexBytes(bytes,n);}
inline bool secureEqual(const std::string& a,const std::string& b){if(a.size()!=b.size())return false;volatile unsigned diff=0;for(size_t i=0;i<a.size();++i)diff|=uint8_t(a[i])^uint8_t(b[i]);return diff==0;}
inline std::string passwordHash(const std::string& password,const std::string& salt){uint8_t out[32];int rc=mbedtls_pkcs5_pbkdf2_hmac_ext(MBEDTLS_MD_SHA256,(const unsigned char*)password.data(),password.size(),(const unsigned char*)salt.data(),salt.size(),100000,32,out);return rc?"":hexBytes(out,32);}
