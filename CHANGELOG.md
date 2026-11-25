# 更新日志

本项目所有重要变更均记录于此文件。格式参考 [Keep a Changelog](https://keepachangelog.com/zh-CN/1.1.0/)，版本号遵循 [语义化版本](https://semver.org/lang/zh-CN/)。

## [1.0.0] - 2026-11-25

### 新增

- 开源首个正式版本，项目归属与作者统一为 **Felix**。
- 完整文档体系：README、功能说明、目录结构、运行与部署、依赖配置、使用示例、注意事项、架构与流程图。
- 系统架构图、3σ 判异原理图、学习/检测模式流程图。
- 脱敏示例数据：`sample_signal.csv`、`sample_model_result.csv`。
- MIT 开源许可、贡献指南与 `.gitignore`。

### 变更

- 将用于外部同步的 Windows 命名事件由历史命名统一改为 `FELIX_WELD_SIGNAL_001`，去除非项目相关的品牌标识。
- 清理工程配置中的本机绝对路径，改为可移植的 `$(QTDIR)` / `$(ProjectDir)` 变量。
- 裁剪原始资料中的大体积二进制测试数据与第三方文档，仅保留核心源码、图标与示例数据。

### 说明

- 核心算法与 UI 逻辑保持与原始工程一致；当前版本仍为工程原型，现场联调与统计实现的健壮性改进见 [注意事项](docs/06-notes.md)。

[1.0.0]: https://github.com/felixccty/WeldingDiagnosis/releases/tag/v1.0.0
