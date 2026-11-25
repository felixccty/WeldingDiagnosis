# 使用示例

本文档给出典型操作流程与数据格式示例，帮助快速上手与离线复现。

## 1. 典型操作流程

### 1.1 建立模型（学习模式）

1. 启动程序，点击顶部「学习模式」按钮（绿色主题）；
2. 点击「添加模型」按钮（带 `+` 图标），在弹窗中填写：
   - 模型名称：如 `model_A`；
   - 样本数量：如 `5`；
3. 在模型表格中选中该模型，使其成为当前模型（`curr_learn_id`）；
4. 触发一次采样（点击「开始学习」或通过外部事件/TCP）；
5. 采样结束后，程序自动统计分析，绿色容差带出现，`tableWidget_sample` 增加一行时间记录；
6. 重复采样直至达到样本数量。

### 1.2 在线检测（检测模式）

1. 点击顶部「检测模式」按钮（蓝色主题）；
2. 确认「采集卡」状态灯为绿（连接成功）；
3. 点击「开始检测」，实时信号曲线滚动显示；
4. 观察实时曲线与学习阶段建立的容差带关系；
5. 点击「停止」结束采集。

### 1.3 外部触发

**方式 A：Windows 事件**

使用任意工具创建/置位名为 `FELIX_WELD_SIGNAL_001` 的事件即可触发一次学习切换。程序内对应 `send_control_signal()`：

```cpp
static HANDLE s_hEnventAquire = INVALID_HANDLE_VALUE;
if (s_hEnventAquire == INVALID_HANDLE_VALUE) {
    s_hEnventAquire = ::CreateEvent(NULL, FALSE, FALSE, L"FELIX_WELD_SIGNAL_001");
}
if (s_hEnventAquire != INVALID_HANDLE_VALUE) {
    ::SetEvent(s_hEnventAquire);
}
```

**方式 B：TCP**

向 `127.0.0.1:12345` 发送 JSON：

```json
{"Start":"01"}
```

服务端解析到 `Start` 为 `"01"` 或 `"02"` 时调用 `start_learn()`，并返回确认 JSON：

```json
[
  {
    "SendTime": "2023 - 10 - 18 11:02 : 35.143",
    "SNCode": "",
    "Result": "OK",
    "NgCode": "",
    "Start": "01",
    "ListIndex": "",
    "ConfigIndex": "1"
  }
]
```

> IP、端口与事件名均在源码中定义，联调时按现场约定修改并同步更新 [依赖配置](04-dependencies.md)。

## 2. 数据格式示例

### 2.1 原始信号 `Data/test.csv`

每次学习追加一行，为逗号分隔的采样点序列，无表头，数值为 AD 采样值：

```csv
2050,2050,2050,2051,2050,2049,2050,2050,2051,2049,...
2050,2050,2049,2050,2049,2051,2050,2050,2048,2050,...
2049,2050,2050,2050,2051,2049,2048,2050,2050,2049,...
```

仓库示例：`examples/data/sample_signal.csv`。

### 2.2 模型结果 `Data/test_result.csv`

共 5 行，每行对应一种统计量，点数与采样长度一致：

| 行号 | 含义 | 生成函数参数 |
|------|------|--------------|
| 1 | 累加和 | `write_data_t`（`total_H`） |
| 2 | 逐点均值 | `write_data_a`（`avg_H`） |
| 3 | 逐点标准差 | `write_data_s`（`standardD_H`） |
| 4 | 下限 `μ-3σ` | `write_data_l`（`listH_low`） |
| 5 | 上限 `μ+3σ` | `write_data_h`（`listH_high`） |

示例（节选）：

```csv
4096,4097,4097,4096,4098,4097,...
4096,4097,4097,4096,4098,4097,...   # 均值
2048,2048,2049,2048,2049,2048,...
-2048,-2047,-2050,-2048,-2050,...   # 下限
10240,10241,10244,10240,10244,...   # 上限
```

仓库示例：`examples/data/sample_model_result.csv`。

### 2.3 离线绘制示例（Python）

借助示例数据可离线复现容差带：

```python
import csv
import matplotlib.pyplot as plt

with open("examples/data/sample_model_result.csv", encoding="ascii") as f:
    rows = [list(map(int, line.strip().split(","))) for line in f if line.strip()]

avg, std, low, high = rows[1], rows[2], rows[3], rows[4]
x = range(len(avg))

plt.plot(x, avg, label="mean", color="#1966ff")
plt.fill_between(x, low, high, color="#3cc63c", alpha=0.35, label="mu +/- 3sigma")
plt.legend()
plt.xlabel("sample index")
plt.ylabel("AD value")
plt.show()
```

> 说明：示例数据为脱敏截断样本，仅用于理解格式；实际点数以现场采样配置为准。

## 3. 界面操作速查

| 操作 | 效果 |
|------|------|
| 拖动标题栏 | 移动无边框窗口 |
| 点击「学习模式」/「检测模式」 | 切换界面与主题色 |
| 点击「添加模型」 | 弹出新建模型对话框 |
| 点击「设置」 | 显示/隐藏基础设置面板 |
| 悬停「采集卡状态」按钮 | 显示连接状态面板 |
| 点击「开始学习」 | 通过事件触发学习流程 |
| 点击「开始检测」/「停止」 | 开始/停止实时采集 |
| 图表区域框选 | 缩放查看细节 |
