# SysTrayMeter Modern 1.0.5（CPU 专用宽字版）

独立重写的 C++ / Win32 托盘 CPU、内存监视器。面向 Windows 10 / 11 x64。

已嵌入用户提供的 EXE 图标，文件属性公司为 `Computers are fun, Yes!`；文件和产品版本为 1.0.5。CPU 使用 4 像素宽数字，内存保持 3 像素宽；字间空白均为 1 像素，CPU 右侧边距 2 像素，内存右侧边距 1 像素。

## 使用

1. 解压到一个固定目录，退出旧版 SysTrayMeter。
2. 双击 SysTrayMeterModern.exe。无需安装、无需管理员权限，也无需安装 .NET 或 VC++ 运行库。
3. Windows 可能把图标放进托盘折叠区，拖出来即可。

默认上区 CPU 已用百分比，下区可用物理内存。第一次 CPU 采样约需一秒，期间显示 `--`。
悬停显示 CPU、统计方式、内存已用/总量及可用量。双击退出，保留旧版习惯。
右键菜单提供编辑配置、重新加载配置、退出。没有自动创建开机启动项；需要时可手动在启动文件夹放快捷方式。

## 内存显示

全程使用 Windows GlobalMemoryStatusEx 的 64 位物理内存数据；可用量包含可回收的内存，与 Windows“可用”口径对应，不是只统计完全空闲页。

- 0～9999 MiB：显示整数，例如 `7000`。
- 超过四位：仅显示末四位，例如 `12345 → 2345`、`10000 → 0000`、`65536 → 5536`。
- 不添加单位字母；悬停仍显示完整内存信息。

上下两区均右对齐，CPU 右侧保留 2 像素边距，内存保留 1 像素边距。采用专用纯数字像素字，在 7 / 9 像素分区中分别绘制为 5 / 7 像素高，上下各留 1 像素；四位数字占 15 像素宽。无数据时仍用横线提示。高 DPI 按系统托盘小图标尺寸绘制，最终观感需在 Windows 实测。

## CPU 统计

默认读取英文路径 `\Processor Information(_Total)\% Processor Utility`，避免中文 Windows 的计数器名称差异。它是系统总计数器，不把某个核心的占用当成整机占用，也不手工重复除以线程数。

若 Utility 不可用，则回退到 `Processor Information` 的 `% Processor Time`；再尝试 `Processor(_Total)`。悬停信息注明实际使用的方式。若计数器无法读取，则显示 `--`，不会伪装成 0%。

Utility 和传统 Time 是不同统计口径，频率变化和采样时间会影响结果，不能保证与任务管理器每秒完全相同。配置 `CpuMetric=1` 可强制使用传统 Time，方便对照。此版本尚未在真实 13600K/5800H 上验证。

## 配置

同目录 SysTrayMeter.ini。附带文件保留用户上传的配色；旧版同名配置可直接使用。

[Settings]
- ReverseOrder=0：上区高 7，下区高 9；1：上区 9，下区 7。
- InfoSection=0：CPU 在上，内存在下；1：交换。
- ShowCpu=0：无数字；1：空闲百分比；2：已用百分比。
- ShowUsedRam=0：无数字；1：可用内存；2：已用内存。
- UpdateIntervalMs=1000：刷新毫秒数，允许 250～60000。
- CpuMetric=0：优先 Utility，失败时回退 Time；1：使用 Time。

[CpuColors] / [RamColors]
- NbText / NbBack：文字/背景颜色规则数量（最多 64）。
- Text1.MaxValue / Back1.MaxValue：该颜色的最高占用百分比。
- Text1.Color / Back1.Color：#RRGGBB。
- 后续规则依次 Text2、Text3……或 Back2、Back3……。
- 阈值始终按“已用百分比”判断，即使数字显示可用量。

修改后在托盘右键选择“重新加载配置”。读取路径固定为 EXE 所在目录。编辑菜单会用系统的 INI 默认关联程序打开该文件。

## 编译源码（仅修改源码时需要）

已附编译好的 x64 EXE，普通使用不需要工具链。

- Windows MSVC：安装 Visual Studio C++ 工具，在 x64 Native Tools Command Prompt 中运行 build_msvc.cmd。
- Linux MinGW-w64：安装 g++-mingw-w64-x86-64-posix，然后运行 `sh build_mingw.sh`。
- 核心格式测试：`g++ -std=c++17 src/tests.cpp -o tests && ./tests`。

源码在 src/，无第三方源代码依赖。发布版静态链接 GCC / C++ 运行库，系统依赖仅为 Windows 自带 DLL。

## 已做检查与实测范围

已在 Linux 执行内存边界/大容量/文本宽度/百分比边界测试。Windows EXE 的编译结果和导入表检查见 BUILD_INFO.txt。
当前环境不能运行真实 Windows 托盘，未宣称已完成 Windows 运行验收。

请重点检查：
1. 右对齐、大字体及末四位可用内存显示；32/64 GiB 内存下悬停数值。
2. 空闲、单线程负载、多线程负载时，与任务管理器的整体趋势是否一致。
3. 100% CPU 能否完整显示；缩放 100%/150%/200% 时是否可读。
4. 重新加载 INI、Explorer 重启后图标恢复、睡眠唤醒后采样恢复。

## 来源说明

本程序从需求独立实现，并非对旧 EXE 的修改或反编译产物。UI 概念与 INI 键名参考 jacky 于 2007 年发布的 SysTrayMeter；附带配色来自用户提供的配置。旧版二进制不在本包中。
