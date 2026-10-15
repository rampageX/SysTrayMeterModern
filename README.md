# SysTrayMeter Modern

**一个托盘图标，同时看 CPU 和内存。**

SysTrayMeter Modern 是用 C++ / Win32 独立重写的轻量 Windows 托盘监视器，灵感来自 2007 年的 SysTrayMeter。保留原版在 16×16 小图标中上下分区显示的设计，使用 64 位内存统计和 Windows CPU 性能计数器，适应大容量内存和现代多核处理器。

无需安装，不依赖 .NET 或额外的 VC++ 运行库。默认上区显示 CPU 已用百分比，下区显示可用物理内存；把鼠标放上去即可查看完整信息。

## 特点

- **小巧直观**：一个图标显示两项指标，没有常驻仪表盘窗口。
- **专用像素数字**：上下右对齐，右侧留 1 像素；默认上区字高 5 像素、下区 7 像素，上下各留 1 像素。
- **按真实负载变色**：文字和背景分别支持多级阈值配色。
- **大内存支持**：64 位容量统计，托盘数字采用末四位显示，悬停查看完整数值。
- **整机 CPU 统计**：优先使用 Processor Utility，可切换传统 Processor Time。
- **INI 配置**：支持交换上下内容、调整分区大小、显示已用或剩余量、自定义颜色及刷新间隔。
- **便携运行**：配置位于 EXE 同目录，读取路径不受启动时工作目录影响。
- **状态恢复**：包含 Explorer 重启后重新添加图标、睡眠唤醒后重建 CPU 采样的处理。

## 运行环境

Windows 10 / 11，x64。普通用户权限即可运行。

## 快速开始

1. 解压程序，保留 `SysTrayMeterModern.exe` 和 `SysTrayMeter.ini` 在同一目录。
2. 如正在使用旧版，先退出旧版。
3. 运行 `SysTrayMeterModern.exe`。
4. 若图标出现在托盘折叠区，将它拖到任务栏可见区域。

| 操作 | 功能 |
| --- | --- |
| 鼠标悬停 | 查看 CPU 百分比、实际统计方式，以及内存已用量、总量和可用量 |
| 右键 → 编辑配置 | 使用系统关联程序打开 INI |
| 右键 → 重新加载配置 | 应用修改，无需重启程序 |
| 双击图标 / 右键 → 退出 | 退出程序 |

首次 CPU 采样约需一个刷新周期；采样尚未就绪或读取失败时显示 `--`。

如需开机启动，按 `Win + R`，输入 `shell:startup`，将 EXE 的快捷方式放入打开的文件夹。

## 数字和颜色规则

### CPU

显示整数百分比，不添加 `%`，以保留有限的图标空间。默认显示已用比例，也可配置为空闲比例。

### 内存

图标按整数 **MiB** 显示（1 MiB = 1,048,576 字节），不添加单位字母。超过四位时，**只保留十进制末四位，并保留前导零**。

| 实际可用内存 | 图标显示 |
| --- | --- |
| 7000 MiB | `7000` |
| 9999 MiB | `9999` |
| 10000 MiB | `0000` |
| 12345 MiB | `2345` |
| 16384 MiB | `6384` |
| 65536 MiB | `5536` |

这一规则用于观察变化和趋势；跨越万位时数字会回绕。悬停提示仍显示未截位的内存信息，容量以 GiB 表示、保留两位小数。

内存数据由 `GlobalMemoryStatusEx` 获取。“可用”包含系统可回收的内存，不只是完全空闲的页面。

### 动态配色

**CPU 和内存的文字、底色均按真实的已用百分比判断。** 即使数字显示空闲 CPU、可用内存，或内存数字截取了末四位，配色仍按实际已用比例变化。

附带配置中，CPU 使用率超过 50% 时切换为白字红底；内存使用率超过 80% 时切换为白字黑底。可在 INI 中自行调整。

## 配置

编辑 EXE 同目录的 `SysTrayMeter.ini`，保存后右键选择“重新加载配置”。

### 基本设置

```ini
[Settings]
UpdateIntervalMs=1000
CpuMetric=0
ReverseOrder=0
InfoSection=0
ShowCpu=2
ShowUsedRam=1
```

| 配置项 | 可选值与含义 |
| --- | --- |
| `UpdateIntervalMs` | 刷新间隔，单位毫秒，范围 250～60000，默认 1000 |
| `CpuMetric` | `0`：优先 Utility，失败时回退 Time；`1`：使用 Time |
| `ReverseOrder` | `0`：上区高 7、下区高 9；`1`：上区高 9、下区高 7 |
| `InfoSection` | `0`：上 CPU、下内存；`1`：上内存、下 CPU |
| `ShowCpu` | `0`：隐藏数字；`1`：空闲百分比；`2`：已用百分比 |
| `ShowUsedRam` | `0`：隐藏数字；`1`：可用内存；`2`：已用内存 |

### 配色设置

`[CpuColors]` 控制 CPU，`[RamColors]` 控制内存。文字与背景独立配置，例如：

```ini
[CpuColors]
NbText=2
Text1.MaxValue=50
Text1.Color=#000000
Text2.MaxValue=100
Text2.Color=#FFFFFF

NbBack=2
Back1.MaxValue=50
Back1.Color=#FFFFFF
Back2.MaxValue=100
Back2.Color=#FF0000
```

- `NbText` / `NbBack`：文字 / 背景规则数量，最多 64 条。
- `MaxValue`：该规则覆盖的最高已用百分比，包含该值。
- `Color`：`#RRGGBB` 格式的颜色。
- 按阈值从小到大排列规则，建议最后一条设为 `100`。

上例中，CPU 已用比例不超过 50% 时为黑字白底，超过 50% 时为白字红底。项目沿用原版的主要 INI 键名，可参考附带配置迁移原有配色。

## CPU 统计说明

默认使用 Windows PDH 英文计数器路径：

```text
\Processor Information(_Total)\% Processor Utility
```

使用系统总计数器汇总整机负载，不将单个核心的数值作为整机占用，也不重复除以线程数。英文路径避免依赖本地化计数器名称。

Utility 不可用时，尝试 `Processor Information(_Total)` 的 `% Processor Time`，再尝试 `Processor(_Total)` 的同类计数器。悬停提示显示实际采用的统计方式。

Utility 与 Time 的统计口径不同，频率变化、采样时刻与刷新间隔也会影响数值，因此不保证每次刷新都与任务管理器完全一致。需要对照传统 CPU 时间占用时，可设置 `CpuMetric=1`。

## 从源码编译

普通使用直接运行编译好的 EXE 即可。源码使用 C++17 和 Windows 系统 API，无第三方源代码依赖。

### Windows：MSVC

安装 Visual Studio 或 Build Tools 的 C++ 桌面开发工具，包含 Windows SDK。打开 **x64 Native Tools Command Prompt for VS**，进入项目目录后执行：

```bat
build_msvc.cmd
```

### Linux：MinGW-w64

安装提供 `x86_64-w64-mingw32-g++` 和 `x86_64-w64-mingw32-windres` 的工具链，在项目目录执行：

```sh
sh build_mingw.sh
```

两种方式均生成 `SysTrayMeterModern.exe`。MSVC 使用静态运行库；MinGW 静态链接 GCC / C++ 运行库，发布程序仅依赖 Windows 自带 DLL。

### 核心测试

使用本机 C++17 编译器运行数字格式和百分比边界测试：

```sh
g++ -std=c++17 src/tests.cpp -o tests
./tests
```

测试覆盖四位显示边界、截位后补零、大容量输入、纯数字及宽度限制、百分比范围。真实托盘、DPI 和 CPU 计数器行为需要在 Windows 上验证。

## 项目文件

| 路径 | 用途 |
| --- | --- |
| `src/main.cpp` | 托盘交互、采样、INI 配置与像素数字绘制 |
| `src/meter_core.h` | 内存数字格式和百分比处理 |
| `src/tests.cpp` | 核心边界测试 |
| `src/app.rc` | 图标和文件版本资源 |
| `src/app.manifest` | 权限、系统兼容性与 DPI 声明 |
| `src/SysTrayMeter.ico` | EXE 图标 |
| `SysTrayMeter.ini` | 默认配置与配色 |
| `build_msvc.cmd` / `build_mingw.sh` | 编译脚本 |
| `BUILD_INFO.txt` | 随包 EXE 的构建信息和 SHA-256 |

## 显示与反馈

图标采用 16×16 逻辑画布，按系统小图标尺寸放大绘制。混合 DPI 多屏及不同缩放比例下的观感可随环境而变化。

提交问题时，请附上 Windows 版本、屏幕缩放比例、相关 INI 设置及截图。涉及 CPU 读数时，同时注明 CPU 型号、悬停显示的统计方式和对照时的负载情况。

## 致谢与来源

感谢 jacky 于 2007 年发布的 SysTrayMeter 带来的精巧设计。本项目参考其上下分区的显示概念和主要 INI 键名，以 C++ / Win32 独立实现，并非修改或反编译旧版 EXE。配色沿用项目发起者提供的配置，EXE 图标由项目发起者提供。

**Computers are fun, Yes!**
