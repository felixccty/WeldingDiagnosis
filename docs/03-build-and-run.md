# 运行与部署

本文档说明从源码编译到现场部署的完整流程。

## 1. 前置条件

- Windows 7 / 10 x64；
- Visual Studio 2019（工作负载「使用 C++ 的桌面开发」，工具集 `v142`）；
- Qt 5.12.x + Qt VS Tools（Qt 组件见 [依赖配置](04-dependencies.md)）；
- USB9825 采集卡、厂商驱动与 SDK；
- （可选）Qt 命令行工具 `windeployqt`，用于生成发布目录。

## 2. 获取采集卡 SDK

源码通过 `#include "USB9825.h"` 并链接 `USB9825.lib` 使用采集卡 API。仓库出于许可考虑未附带厂商二进制文件，请按以下步骤准备：

1. 从采集卡供应商处获取 USB9825 SDK（含 `USB9825.h`、`USB9825.lib`、`USB9825.dll` 及驱动）；
2. 将 `USB9825.h` 与 `USB9825.lib` 放入 `src/` 目录（头文件可用仓库 `third_party/USB9825.h` 对照）；
3. 确认 `WeldingDiagnosis.vcxproj` 中 `ClInclude`/`Library`/`None` 三项已指向上述文件（工程已预置）。

> 仓库中的 `third_party/USB9825.h` 仅作为接口参考副本，内容与厂商 SDK 头文件一致。

## 3. 编译

1. 用 Visual Studio 2019 打开 `src/WeldingDiagnosis.sln`；
2. 在「解决方案配置」中选择 `Debug` 或 `Release`，平台选择 `x64`；
3. 在 Qt VS Tools 中确认 Qt 版本为 `msvc2017_64`（工程 `QtInstall` 字段）；
4. 生成解决方案（`Ctrl+Shift+B`）。

输出：`x64/Debug/WeldingDiagnosis.exe` 或 `x64/Release/WeldingDiagnosis.exe`。

### 常见问题

- **找不到 Qt 头文件 / 链接错误**：确认 Qt VS Tools 已配置 `msvc2017_64` 套件，且工程 Qt Modules 包含 `charts;core;gui;network;widgets`。
- **找不到 `USB9825.lib`**：确认第 2 步已将库文件放入 `src/`。
- **`QtMsBuild` 提示找不到 `qt.targets`**：在 VS 扩展菜单中执行 Qt VS Tools 的版本设置，使其写入工程目录。

## 4. 目录部署

程序运行时依赖可执行文件同目录下的 `ico/` 图标与 `USB9825.dll`。推荐部署结构：

```
WeldingDiagnosis/
├── WeldingDiagnosis.exe
├── USB9825.dll
└── ico/
    ├── logo.png
    ├── 设置.png
    ├── 最小化.png
    ├── 关闭.png
    ├── 添加模型.png
    ├── 学习模式.png
    ├── 学习模式点亮.png
    ├── 检测模式.png
    ├── 检测模式闭眼.png
    └── 焊接.png
```

操作步骤：

1. 新建发布目录，放入 `WeldingDiagnosis.exe` 与 `USB9825.dll`；
2. 将本仓库 `resources/ico/` 复制为发布目录下的 `ico/`；
3. 安装采集卡驱动，插入设备后启动程序。

程序会在同目录自动创建 `Data/` 存放 CSV。

## 5. 使用 windeployqt 打包

仓库 `docs/build-command.txt` 记录了打包命令，核心形式：

```
windeployqt 可执行程序名
```

例如在 Qt 的 MSVC 命令行中：

```
windeployqt C:\Deploy\WeldingDiagnosis\WeldingDiagnosis.exe
```

该命令会复制所需的 Qt 运行时 DLL 与插件目录（如 `platforms/`、`imageformats/`、`iconengines/`）。执行后再手动补入 `USB9825.dll` 与 `ico/` 目录。

> 详细命令与原始记录见 [build-command.txt](build-command.txt)。

## 6. 验证运行

启动后应观察到：

1. 显示无边框主界面，标题栏可拖动；
2. 「采集卡」状态灯由红变绿（连接成功时按钮文字显示「连接成功」）；
3. 切换学习/检测模式，界面主题色在绿/蓝之间切换，图表正常显示；
4. 点击「开始检测」后实时曲线滚动更新；
5. `Data/` 目录在首次学习后生成 CSV 文件。

若状态灯持续为红，请检查驱动安装、设备连接与设备号占用情况（程序轮询 0~31，连接第一块可用设备）。
