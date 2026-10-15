#pragma once
#include <string>
#include <cwchar>
#include <cmath>
#include <cstdint>
#include <algorithm>
namespace meter {
inline std::wstring memoryText(uint64_t bytes) {
 const uint64_t mib=bytes/(1024ULL*1024);
 if(mib<=9999) return std::to_wstring(mib);
 // Keep the last four decimal MiB digits, including leading zeroes.
 wchar_t b[5];
 swprintf(b,5,L"%04u",unsigned(mib%10000));
 return b;
}
inline int percent(double v) {return int(std::lround(std::clamp(v,0.0,100.0)));}
}
