#include "meter_core.h"
#include <cassert>
#include <iostream>
int main(){
 constexpr uint64_t M=1024ULL*1024;
 assert(meter::memoryText(0)==L"0");
 assert(meter::memoryText(7000*M)==L"7000");
 assert(meter::memoryText(9999*M)==L"9999");
 assert(meter::memoryText(10000*M)==L"0000");
 assert(meter::memoryText(12345*M)==L"2345");
 assert(meter::memoryText(10001*M)==L"0001");
 assert(meter::memoryText(65536*M)==L"5536");
 assert(meter::memoryText(UINT64_MAX)==L"4415");
 for(uint64_t mb=0;mb<2*1024*1024;mb+=17){
  auto s=meter::memoryText(mb*M);
  assert(s.size()<=4);
  if(mb>9999)assert(s.size()==4);
  for(auto c:s)assert(c>=L'0'&&c<=L'9');
  assert(std::stoul(s)==mb%10000);
 }
 assert(meter::percent(1000)==100);assert(meter::percent(-3)==0);assert(meter::percent(49.6)==50);
 std::cout<<"PASS: last-four-digit formatting, zero padding, 64-bit input, numeric-only width and percentage bounds\n";
}
