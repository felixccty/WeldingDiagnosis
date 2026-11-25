# 目录结构

本文档说明开源发布包各目录与文件的职责，以及源码内的模块组织关系。

## 1. 仓库目录树

```
WeldingDiagnosis/
├── README.md                     # 项目总览
├── LICENSE                       # MIT 许可（Copyright Felix）
├── CHANGELOG.md                  # 版本记录
├── CONTRIBUTING.md               # 贡献指南
├── .gitignore                    # 忽略规则
├── src/                          # 核心源码
│   ├── main.cpp
│   ├── WeldingDiagnosis.h
│   ├── WeldingDiagnosis.cpp
│   ├── WeldingDiagnosis.ui
│   ├── WeldingDiagnosis.qrc
│   ├── WeldingDiagnosis.sln
│   ├── WeldingDiagnosis.vcxproj
│   └── WeldingDiagnosis.vcxproj.filters
├── third_party/
│   └── USB9825.h                 # 采集卡 SDK 头文件（参考副本）
├── resources/ico/                # 界面图标（PNG）
├── examples/data/                # 脱敏示例数据
│   ├── sample_signal.csv
│   └── sample_model_result.csv
├── docs/
│   ├── 01-features.md
│   ├── 02-structure.md
│   ├── 03-build-and-run.md
│   ├── 04-dependencies.md
│   ├── 05-usage.md
│   ├── 06-notes.md
│   ├── 07-architecture.md
│   ├── build-command.txt
│   └── images/
│       ├── architecture.svg
│       ├── learning-flow.svg
│       ├── detection-flow.svg
│       └── signal-principle.svg
└── (运行/本地按需) USB9825.dll, USB9825.lib, ico/, Data/
```

## 2. 源码文件职责

| 文件 | 职责 |
|------|------|
| `src/main.cpp` | Qt 应用入口，构造并展示主窗口 |
| `src/WeldingDiagnosis.h` | 声明 `WeldingDiagnosis` 主窗口类，以及 `thread_obj`（学习采样线程）、`thread_ctl`（控制线程）两个 `QObject` |
| `src/WeldingDiagnosis.cpp` | 全部业务实现：初始化、双模式、采集、统计、CSV、TCP、事件同步 |
| `src/WeldingDiagnosis.ui` | Qt Designer 生成的界面 XML（控件布局、样式表） |
| `src/WeldingDiagnosis.qrc` | Qt 资源集合，当前为空占位 |
| `src/WeldingDiagnosis.sln` | Visual Studio 解决方案 |
| `src/WeldingDiagnosis.vcxproj` | 工程配置：Qt 模块、平台工具集 `v142`、编译文件清单 |
| `src/WeldingDiagnosis.vcxproj.filters` | VS 筛选器分类 |
| `third_party/USB9825.h` | 采集卡接口声明：初始化结构体、触发模式/触发源枚举、API 函数 |

## 3. 类与线程关系

- **主线程 `WeldingDiagnosis`**：持有 UI 与全部图表对象，负责状态更新与统计；
- **`thread_obj`**：在独立 `QThread` 中运行 `learning_thread()`，循环采样并通过 `signal_H(int)` 发往主线程；
- **`thread_ctl`**：在独立 `QThread` 中运行 `control_thread()`，等待 Windows 事件并通过 `control_signal(int)` 发往主线程；
- **多个 `QTimer`**：1 ms 状态刷新、1 ms 实时曲线、10 ms 统计分析。

模块关系详见 [架构与流程](07-architecture.md)。

## 4. 图标资源

`resources/ico/` 中的 PNG 由 `ico_initial()` 在运行时按可执行文件路径加载，代码中的引用与文件对应如下：

| 文件 | 用途 |
|------|------|
| `logo.png` | 窗口图标 |
| `设置.png` | 设置按钮 |
| `最小化.png` / `关闭.png` | 窗口控制按钮 |
| `添加模型.png` | 新增模型按钮 |
| `学习模式.png` / `学习模式点亮.png` | 学习模式按钮常态/按下 |
| `检测模式.png` / `检测模式闭眼.png` | 检测模式按钮常态/按下 |
| `焊接.png` | 标题栏标识 |

> 部署时请将 `resources/ico/` 复制到可执行文件同目录并命名为 `ico/`，否则图标将无法显示。

## 5. 数据文件

- `examples/data/sample_signal.csv`：脱敏后的原始采样序列示例（每行一帧，逗号分隔）；
- `examples/data/sample_model_result.csv`：脱敏后的模型统计结果示例（5 行统计量）。

运行时产生的 `Data/` 目录由程序自动创建，不纳入版本管理。

## 6. 裁剪说明

以下原始资料未纳入开源发布包，以保持仓库精简：

- `代码备份/`、`Debug/`、`Release/`、`x64/` 等历史与构建产物；
- `打包/` 中的 Qt 运行时 DLL 与可执行文件（可通过 `windeployqt` 重新生成）；
- `文档/` 中的第三方培训资料、专利文献、采集卡 SDK 光盘、大体积 `.bin` 测试数据；
- USB9825 厂商二进制 `.dll/.lib`（受第三方许可约束，需自行获取）。
