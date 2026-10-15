#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <shellapi.h>
#include <pdh.h>
#include <pdhmsg.h>
#include <vector>
#include <string>
#include <cwchar>
#include <algorithm>
#include "meter_core.h"

namespace {
constexpr UINT TrayMessage=WM_APP+1;
std::wstring iniPath;
struct Rule {int max; COLORREF color;};
struct Palette {std::vector<Rule> text,back;};
struct Config {int reverse=0,info=0,cpu=2,ram=1,interval=1000,mode=0; Palette cp,rp;} cfg;
HQUERY query=nullptr; HCOUNTER utility=nullptr,timeCounter=nullptr;
NOTIFYICONDATAW nid{}; UINT taskbarCreated; HICON currentIcon=nullptr;
MEMORYSTATUSEX mem{}; bool memOK=false,cpuOK=false; double cpu=0; std::wstring cpuSource=L"Sampling";
int readInt(const wchar_t* section,const wchar_t* key,int def) {return int(GetPrivateProfileIntW(section,key,def,iniPath.c_str()));}
COLORREF readColor(const wchar_t* section,const std::wstring& key,COLORREF fallback) {
 wchar_t b[32]; GetPrivateProfileStringW(section,key.c_str(),L"",b,32,iniPath.c_str());
 unsigned r,g,bl; if(wcslen(b)==7 && swscanf(b,L"#%2x%2x%2x",&r,&g,&bl)==3) return RGB(r,g,bl);
 return fallback;
}
Palette loadPalette(const wchar_t* section,bool ram) {
 Palette p; for(int kind=0;kind<2;++kind) {
 auto& rules=kind?p.back:p.text;
 const wchar_t* prefix=kind?L"Back":L"Text";
 const int count=std::clamp(readInt(section,kind?L"NbBack":L"NbText",0),0,64);
 for(int i=1;i<=count;++i) {auto key=std::wstring(prefix)+std::to_wstring(i);rules.push_back({std::clamp(readInt(section,(key+L".MaxValue").c_str(),100),0,100),readColor(section,key+L".Color",kind?RGB(0,128,0):RGB(0,0,0))});}
 std::stable_sort(rules.begin(),rules.end(),[](const Rule&a,const Rule&b){return a.max<b.max;});
 if(rules.empty()) {if(kind) rules={{ram?80:50,ram?RGB(0,128,0):RGB(255,255,255)},{100,ram?RGB(0,0,0):RGB(255,0,0)}};else rules={{ram?80:50,RGB(0,0,0)},{100,RGB(255,255,255)}};}
 } return p;
}
void loadConfig() {
 cfg.reverse=readInt(L"Settings",L"ReverseOrder",0)!=0;cfg.info=readInt(L"Settings",L"InfoSection",0)!=0;
 cfg.cpu=std::clamp(readInt(L"Settings",L"ShowCpu",2),0,2); cfg.ram=std::clamp(readInt(L"Settings",L"ShowUsedRam",1),0,2);
 cfg.interval=std::clamp(readInt(L"Settings",L"UpdateIntervalMs",1000),250,60000);cfg.mode=readInt(L"Settings",L"CpuMetric",0)==1;
 cfg.cp=loadPalette(L"CpuColors",false);cfg.rp=loadPalette(L"RamColors",true);
}
void startCounters() {
 if(query) { PdhCloseQuery(query); }
 query=nullptr;utility=nullptr;timeCounter=nullptr;cpuOK=false;cpuSource=L"Sampling";
 if(PdhOpenQueryW(nullptr,0,&query)!=ERROR_SUCCESS) return;
 if(PdhAddEnglishCounterW(query,L"\\Processor Information(_Total)\\% Processor Utility",0,&utility)!=ERROR_SUCCESS)utility=nullptr;
 if(PdhAddEnglishCounterW(query,L"\\Processor Information(_Total)\\% Processor Time",0,&timeCounter)!=ERROR_SUCCESS) {
  timeCounter=nullptr;PdhAddEnglishCounterW(query,L"\\Processor(_Total)\\% Processor Time",0,&timeCounter);
 }
 PdhCollectQueryData(query);
}
bool readCounter(HCOUNTER c,double& out) {
 if(!c) { return false; }
 PDH_FMT_COUNTERVALUE v{};
 if(PdhGetFormattedCounterValue(c,PDH_FMT_DOUBLE,nullptr,&v)!=ERROR_SUCCESS || (v.CStatus!=PDH_CSTATUS_VALID_DATA && v.CStatus!=PDH_CSTATUS_NEW_DATA) || !std::isfinite(v.doubleValue))return false;
 out=std::clamp(v.doubleValue,0.0,100.0);return true;
}
void sample() {
 mem.dwLength=sizeof(mem);memOK=GlobalMemoryStatusEx(&mem)!=FALSE;cpuOK=false;
 if(query && PdhCollectQueryData(query)==ERROR_SUCCESS) {
  if(!cfg.mode && readCounter(utility,cpu)) {cpuOK=true;cpuSource=L"Processor Utility";}
  else if(readCounter(timeCounter,cpu)) {cpuOK=true;cpuSource=L"Processor Time";}
 }
}
COLORREF colorAt(const std::vector<Rule>& r,int usage) {for(auto& v:r)if(usage<=v.max)return v.color;return r.back().color;}
// Narrow 3x7 master digits; each section reserves a one-pixel vertical inset.
const char* glyph(wchar_t ch) {
 switch(ch) {
 case L'0':return "111101101101101101111";
 case L'1':return "010110010010010010111";
 case L'2':return "111001001111100100111";
 case L'3':return "111001001111001001111";
 case L'4':return "101101101111001001001";
 case L'5':return "111100100111001001111";
 case L'6':return "111100100111101101111";
 case L'7':return "111001001010010010010";
 case L'8':return "111101101111101101111";
 case L'9':return "111101101111001001111";
 // Only a missing-sample indicator; numeric labels contain digits alone.
 case L'-':return "000000000111000000000";
 default:return "000000000000000000000";
 }
}
const char* shortGlyph(wchar_t ch) {
 switch(ch) {
 case L'0':return "111101101101111";case L'1':return "010110010010111";
 case L'2':return "111001111100111";case L'3':return "111001111001111";
 case L'4':return "101101111001001";case L'5':return "111100111001111";
 case L'6':return "111100111101111";case L'7':return "111001010010010";
 case L'8':return "111101111101111";case L'9':return "111101111001111";
 case L'-':return "000000111000000";default:return "000000000000000";
 }
}
void textPixels(COLORREF* px,const std::wstring& s,int y,int height,COLORREF color) {
 const int width=s.empty()?0:int(s.size())*4-1;
 int x=15-width; // One pixel of background at the shared right edge.
 for(auto c:s) {
  const auto g=height==5?shortGlyph(c):glyph(c);
  for(int yy=0;yy<height;++yy)for(int xx=0;xx<3;++xx)
   if(g[yy*3+xx]=='1'&&x+xx>=0&&x+xx<16)
    px[(y+yy)*16+x+xx]=color;
  x+=4;
 }
}
HICON makeIcon() {
 COLORREF px[256];int split=cfg.reverse?9:7;
 for(int section=0;section<2;++section) {
  bool ram=(section==0)?cfg.info:!cfg.info;auto& pal=ram?cfg.rp:cfg.cp;
  int usage=ram?(memOK?int(mem.dwMemoryLoad):0):(cpuOK?meter::percent(cpu):0);
  int y0=section?split:0,y1=section?16:split;
  auto bg=colorAt(pal.back,usage),fg=colorAt(pal.text,usage);
  for(int y=y0;y<y1;++y)for(int x=0;x<16;++x)px[y*16+x]=bg;
  std::wstring label;
  if(ram && cfg.ram)label=memOK?meter::memoryText(cfg.ram==1?mem.ullAvailPhys:mem.ullTotalPhys-mem.ullAvailPhys):L"--";
  if(!ram && cfg.cpu)label=cpuOK?std::to_wstring(meter::percent(cfg.cpu==1?100-cpu:cpu)):L"--";
  textPixels(px,label,y0+1,y1-y0-2,fg);
 }
 int w=std::max(16,GetSystemMetrics(SM_CXSMICON)),h=std::max(16,GetSystemMetrics(SM_CYSMICON));
 BITMAPINFO bi{};bi.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);bi.bmiHeader.biWidth=w;bi.bmiHeader.biHeight=-h;bi.bmiHeader.biPlanes=1;bi.bmiHeader.biBitCount=32;bi.bmiHeader.biCompression=BI_RGB;
 void* bits=nullptr;HBITMAP bitmap=CreateDIBSection(nullptr,&bi,DIB_RGB_COLORS,&bits,nullptr,0);if(!bitmap)return nullptr;
 auto dst=static_cast<DWORD*>(bits);for(int y=0;y<h;++y)for(int x=0;x<w;++x){auto c=px[(y*16/h)*16+x*16/w];dst[y*w+x]=0xff000000u|(DWORD(GetRValue(c))<<16)|(DWORD(GetGValue(c))<<8)|GetBValue(c);}
 HBITMAP mask=CreateBitmap(w,h,1,1,nullptr);if(!mask){DeleteObject(bitmap);return nullptr;}
 HDC dc=CreateCompatibleDC(nullptr);auto old=SelectObject(dc,mask);PatBlt(dc,0,0,w,h,BLACKNESS);SelectObject(dc,old);DeleteDC(dc);
 ICONINFO ii{};ii.fIcon=TRUE;ii.hbmColor=bitmap;ii.hbmMask=mask;HICON icon=CreateIconIndirect(&ii);DeleteObject(mask);DeleteObject(bitmap);return icon;
}
void refreshIcon(bool add=false) {
 auto icon=makeIcon();if(!icon)return;nid.hIcon=icon;nid.uFlags=NIF_ICON|NIF_MESSAGE|NIF_TIP;
 std::wstring cpuStr=cpuOK?std::to_wstring(meter::percent(cpu))+L"%":L"--";
 if(memOK) swprintf(nid.szTip,128,L"CPU: %ls (%ls)\nRAM: %.2f / %.2f GiB used (%lu%%)\nAvailable: %.2f GiB",cpuStr.c_str(),cpuSource.c_str(),(mem.ullTotalPhys-mem.ullAvailPhys)/1073741824.0,mem.ullTotalPhys/1073741824.0,mem.dwMemoryLoad,mem.ullAvailPhys/1073741824.0);
 else swprintf(nid.szTip,128,L"CPU: %ls\nRAM: unavailable",cpuStr.c_str());
 if(!Shell_NotifyIconW(add?NIM_ADD:NIM_MODIFY,&nid)&&!add)Shell_NotifyIconW(NIM_ADD,&nid);
 if(currentIcon) { DestroyIcon(currentIcon); }
 currentIcon=icon;
}
void menu(HWND hwnd) {
 HMENU m=CreatePopupMenu();AppendMenuW(m,MF_STRING,1,L"重新加载配置 / Reload INI");AppendMenuW(m,MF_STRING,2,L"编辑配置 / Edit INI");AppendMenuW(m,MF_SEPARATOR,0,nullptr);AppendMenuW(m,MF_STRING,3,L"退出 / Exit");
 POINT p;GetCursorPos(&p);SetForegroundWindow(hwnd);int cmd=TrackPopupMenu(m,TPM_RETURNCMD|TPM_RIGHTBUTTON,p.x,p.y,0,hwnd,nullptr);DestroyMenu(m);PostMessageW(hwnd,WM_NULL,0,0);
 if(cmd==1) {loadConfig();startCounters();SetTimer(hwnd,1,cfg.interval,nullptr);refreshIcon();}
 if(cmd==2)ShellExecuteW(hwnd,L"open",iniPath.c_str(),nullptr,nullptr,SW_SHOWNORMAL);
 if(cmd==3)DestroyWindow(hwnd);
}
LRESULT CALLBACK windowProc(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp) {
 if(msg==taskbarCreated){refreshIcon(true);return 0;}
 switch(msg) {
 case WM_TIMER:sample();refreshIcon();return 0;
 case TrayMessage:if(lp==WM_RBUTTONUP||lp==WM_CONTEXTMENU)menu(hwnd);else if(lp==WM_LBUTTONDBLCLK)DestroyWindow(hwnd);return 0;
 case WM_POWERBROADCAST:if(wp==PBT_APMRESUMEAUTOMATIC||wp==PBT_APMRESUMESUSPEND){startCounters();refreshIcon();}return TRUE;
 case WM_DISPLAYCHANGE:case WM_SETTINGCHANGE:refreshIcon();return 0;
 case WM_DESTROY:KillTimer(hwnd,1);Shell_NotifyIconW(NIM_DELETE,&nid);if(currentIcon){DestroyIcon(currentIcon);currentIcon=nullptr;}PostQuitMessage(0);return 0;
 }return DefWindowProcW(hwnd,msg,wp,lp);
}
}
int WINAPI wWinMain(HINSTANCE instance,HINSTANCE,PWSTR,int) {
 HANDLE mutex=CreateMutexW(nullptr,FALSE,L"Local\\SysTrayMeterModern_v1");if(!mutex)return 1;if(GetLastError()==ERROR_ALREADY_EXISTS){CloseHandle(mutex);return 0;}
 wchar_t path[32768];DWORD n=GetModuleFileNameW(nullptr,path,32768);if(!n||n>=32768){CloseHandle(mutex);return 1;}
 iniPath=path;iniPath=iniPath.substr(0,iniPath.find_last_of(L"\\/")+1)+L"SysTrayMeter.ini";
 loadConfig();taskbarCreated=RegisterWindowMessageW(L"TaskbarCreated");
 WNDCLASSW wc{};wc.lpfnWndProc=windowProc;wc.hInstance=instance;wc.lpszClassName=L"SysTrayMeterModernWindow";
 if(!RegisterClassW(&wc)){CloseHandle(mutex);return 1;}
 HWND hwnd=CreateWindowExW(0,wc.lpszClassName,L"SysTrayMeter Modern",WS_OVERLAPPED,0,0,0,0,nullptr,nullptr,instance,nullptr);
 if(!hwnd){CloseHandle(mutex);return 1;}
 nid.cbSize=sizeof(nid);nid.hWnd=hwnd;nid.uID=1;nid.uCallbackMessage=TrayMessage;
 startCounters();mem.dwLength=sizeof(mem);memOK=GlobalMemoryStatusEx(&mem)!=FALSE;refreshIcon(true);
 if(!SetTimer(hwnd,1,cfg.interval,nullptr)){DestroyWindow(hwnd);if(query)PdhCloseQuery(query);CloseHandle(mutex);return 1;}
 MSG msg{};int result;while((result=GetMessageW(&msg,nullptr,0,0))>0){TranslateMessage(&msg);DispatchMessageW(&msg);}
 if(query) { PdhCloseQuery(query); }
 CloseHandle(mutex);return result<0?1:0;
}
