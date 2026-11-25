# 依赖配置

本文档列出构建与运行所需的软件、Qt 模块与硬件 SDK 版本信息。

## 1. 软件环境

| 依赖 | 版本 / 说明 | 用途 |
|------|-------------|------|
| Visual Studio | 2019，工具集 `v142` | C++ 编译与工程构建 |
| Windows SDK | `10.0` | 系统 API（事件、文件、时间） |
| Qt | 5.12.x，MSVC 2017 64-bit 套件（`msvc2017_64`） | UI、图表、网络 |
| Qt VS Tools | 与 VS2019 兼容版本 | 在 VS 中集成 qmake/moc/uic/rcc |
| C++ 标准 | C++11 | 语言特性 |

> 工程 `QtInstall` 字段为 `msvc2017_64`；Qt 5.12 官方提供该套件，与 `v142` 工具集兼容。

## 2. Qt 模块

`WeldingDiagnosis.vcxproj` 的 `QtModules` 配置（Debug|x64）为：

```
charts;core;gui;network;websockets;widgets
```

各模块用途：

| 模块 | 用途 |
|------|------|
| `core` | `QObject` / 信号槽 / `QTimer` / `QVector` / 文件 IO |
| `gui` | `QPainter` / `QIcon` / 渐变与画笔 |
| `widgets` | 主窗口、按钮、表格、标签等控件 |
| `charts` | `QChart` / `QSplineSeries` / `QAreaSeries` / `QValueAxis` |
| `network` | `QTcpServer` / `QTcpSocket` / `QJsonDocument` |
| `websockets` | 工程保留模块，当前业务未直接使用 |

> 32 位配置还包含 `3dcore/3danimation/...` 等 3D 模块，为历史遗留设置；推荐统一使用 `x64` 配置。

## 3. 头文件依赖

源码 `#include` 归纳如下：

- Qt：`QtWidgets/QApplication`、`QtCharts`、`QTimer`、`QtNetwork`、`QRandomGenerator`；
- C/C++ 标准：`<thread>`、`<list>`、`<iostream>`、`<fstream>`、`<cmath>`、`<cstdio>`、`<ctime>`；
- 系统/厂商：`<windows.h>`、`USB9825.h`。

## 4. 采集卡 SDK

| 项 | 说明 |
|----|------|
| 设备 | USB9825 数据采集卡 |
| 所需文件 | `USB9825.h`（接口声明）、`USB9825.lib`（导入库）、`USB9825.dll`（运行时） |
| 关键 API | `USB9825_Link`、`USB9825_InitAD`、`USB9825_ReadAD`、`USB9825_StopAD`、`USB9825_SetPulGen`、`USB9825_ExeSoftTrig`、`USB9825_SetDO` / `GetDI` |
| 采样参数 | 16-bit AD；40 MHz 基准时钟；PWM 100 Hz；采样长度 256 点/帧（学习） |

仓库提供 `third_party/USB9825.h` 作为接口参考；二进制库与驱动请从供应商处获取并自行放置，详见 [运行与部署](03-build-and-run.md) 第 2 节。

### 主要 AD 参数（源码默认值）

| 字段 | 学习模式 | 检测模式 |
|------|----------|----------|
| `TriggerMode` | `TRIG_MODE_POST` | `TRIG_MODE_CONTINUE` |
| `TriggerSource` | `TRIG_SRC_INT_RISING` | `TRIG_SRC_INT_RISING` |
| `lSelADClk` | `ADCLK_INT` | `ADCLK_INT` |
| `lChCnt` | 1 | 1 |
| `TriggerLength` | 10 | 1024 |
| `TriggerDelay` | 100 | 100 |
| `TriggerLevel` | 2048 | 2048 |
| `lADFmt` | `ADFMT_STBIN` | `ADFMT_STBIN` |

## 5. 运行时依赖

| 项 | 说明 |
|----|------|
| `USB9825.dll` | 与 exe 同目录 |
| `ico/` 目录 | 界面图标，与 exe 同目录 |
| Qt 运行时 | 由 `windeployqt` 生成的 DLL 与插件目录 |
| 采集卡驱动 | 系统级安装，设备插入后可用 |

## 6. 网络与系统依赖

- TCP Server 监听 `0.0.0.0:12345`，需放行防火墙；
- Windows 命名事件 `FELIX_WELD_SIGNAL_001` 为跨进程同步约定，控制器侧需使用同名事件；
- 源码使用 `#pragma execution_character_set("utf-8")` 保证字符串字面量按 UTF-8 编译。
