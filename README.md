# 无人机地面站上位机（FlightControlPC）

基于 Qt 开发的无人机地面站（Ground Control Station, GCS）上位机界面，通过串口 / 蓝牙接收飞控数据，实时显示姿态、位置、传感器等状态，支持飞行控制参数设置与传感器校准。

## 功能特性

- **串口通信**：串口参数配置与数据收发（`SerialTab` / `SerialThread`）
- **蓝牙通信**：蓝牙连接与数据透传（`BluetoothThread`）
- **飞行状态显示**：实时展示姿态、GPS、传感器等状态（`FlightStatusTab`）
- **姿态仪表盘**：姿态角可视化（`AttitudeDialWidget`）
- **3D 姿态视图**：三维模型实时渲染飞机姿态（`Drone3DView`）
- **姿态波形图**：姿态数据曲线绘制（`AttitudeWaveChart`）
- **飞行控制设置**：控制参数配置（`FlyCtrlSetTab`）
- **传感器校准**：校准流程（`CalibrateTab`）
- **接收机 / GPS / 传感器面板**：`ReceiverPanel` / `GpsInfoPanel` / `SensorDataPanel`

## 界面结构

主窗口包含四个页面：

1. 串口 / 蓝牙通信
2. 飞行状态
3. 飞行控制设置
4. 传感器校准

## 通信协议

上位机与飞控之间采用二进制帧协议通信，**串口和蓝牙共用同一套帧格式与解析逻辑**（统一通过 `sig_dataSort` 信号分发到各页面）。

### 帧格式

```
┌────────┬────────┬────────┬────────┬──────────────┬────────┐
│ 帧头0   │ 帧头1   │ 功能字  │ 数据长度 │ 数据段        │ 校验和  │
│ 1 字节  │ 1 字节  │ FUN    │ LEN     │ LEN 字节     │ SUM    │
└────────┴────────┴────────┴────────┴──────────────┴────────┘
```

- **上传帧**（飞控 → 上位机）：帧头 `0xAA 0xAA`
- **下行帧**（上位机 → 飞控）：帧头 `0xAA 0xAF`
- **功能字 FUN**：标识数据含义（见下表）
- **数据长度 LEN**：数据段字节数
- **校验和 SUM**：从帧头第 0 字节累加到数据段末尾，取低 8 位（模 256）

串口默认参数：`115200`、8 数据位、无校验、1 停止位、无流控。

### 校验和算法

```cpp
uint8_t calcChecksum(uint8_t *data, int len) {
    uint8_t sum = 0;
    for (int i = 0; i < len; i++) sum += data[i];
    return sum;  // 溢出即取低 8 位
}
```

接收端（`SerialThread::dataParse`）以状态机逐字节解析：等帧头 `0xAA` → 等第二个 `0xAA` → 读功能字 → 读长度 → 收数据段 → 校验，通过后发出完整帧 `sig_dataSort`。

### 上传功能字（飞控 → 上位机）

多字节数据均为**大端序**（高字节在前）。

| 功能字 | 含义 | 长度 | 数据段内容 |
|:---:|:---|:---:|:---|
| `0x01` | 姿态角 | 12 | roll/pitch/yaw（int16，÷100 得度）+ 高度 alt（int32）+ 飞行模式 + 解锁状态 |
| `0x02` | 传感器 | 18 | 加速度 x/y/z（int16，÷1000 得 g）+ 陀螺仪 x/y/z（int16，÷100 得 °/s）+ 磁力计 x/y/z（int16） |
| `0x03` | 遥控器数据 | 20 | 油门/偏航/横滚/俯仰 + AUX1~AUX6（各 int16） |
| `0x05` | 电源 | 4 | 电压（uint16，÷100 得 V）+ 电流（uint16，÷100 得 A） |
| `0x07` | 高度传感器 | 6 | 气压高度（int32，÷100 得 m）+ 超声波高度（uint16，÷100 得 m） |
| `0x0A` | 飞行模式应答 | 1 | 当前飞行模式 |
| `0x10~0x15` | PID 回传 | 18 | 6 组 PID，每组 3 组 P/I/D（各 int16） |
| `0xEF` | CHECK 应答 | 2 | 应答功能字 + 校验 |

### 下行功能字（上位机 → 飞控）

| 功能字 | 含义 |
|:---:|:---|
| `0x01` | 读取 PID / 传感器校准命令 |
| `0x02` | 命令集 2（如 `0xA1` 恢复默认参数） |
| `0x04` | 保存 PID |
| `0x09` | 参数设置（参数 ID + 值） |
| `0x0A` | 飞行模式设置 |
| `0x10~0x15` | 写入 PID（6 组，每组 18 字节：3×P/I/D 各 2 字节） |

## 蓝牙通信

上位机通过 BLE（低功耗蓝牙）连接飞控上的蓝牙串口模块（HM-10 / CC2541 等 FFE0 透传模块），连接成功后蓝牙链路与串口**共用同一套帧协议与解析逻辑**。

### 连接流程

1. **搜索设备**：`QBluetoothDeviceDiscoveryAgent` 以 `LowEnergyMethod` 扫描周围 BLE 设备（超时 45 秒）
2. **建立连接**：`QLowEnergyController::createCentral` 作为中央设备连接目标
3. **发现服务**：连接成功后延时 2 秒（等硬件握手稳定）再 `discoverServices`
4. **匹配服务**：查找串口服务 UUID `FFE0`，找不到则判定设备不兼容并断开
5. **加载特征值**：`discoverDetails` 后查找发送/接收特征值
6. **开启 Notify**：向接收特征值的 CCCD 描述符写入 `0x01 0x00`，成功后标记连接建立，开始收发数据

### 服务与特征值

| 项 | UUID | 说明 |
|:---|:---|:---|
| 串口服务 | `0000FFE0-0000-1000-8000-00805F9B34FB` | BLE 串口透传标准服务 |
| 发送特征值 | `0000FFE1-0000-1000-8000-00805F9B34FB` | 上位机 → 飞控，`WriteWithoutResponse` |
| 接收特征值 | `0000FFE2-0000-1000-8000-00805F9B34FB` | 飞控 → 上位机，需支持 Notify |

### 收发

- **发送**：`writeCharacteristic(m_tx, data, WriteWithoutResponse)` 直接写入 FFE1 特征值
- **接收**：飞控数据经 Notify 推送到 FFE2 特征值，`characteristicChanged` 触发后将原始字节发到 `sig_dataSort`，与串口数据走同一套帧解析流程

## 依赖

- Qt 5.12 及以上（开发环境为 Qt 5.15.2 MinGW 64-bit）
- 需要的 Qt 模块 / 组件：
  - Qt Core / GUI / Widgets
  - Qt Charts
  - Qt SerialPort
  - Qt Bluetooth
  - Qt 3D（3DCore / 3DRender / 3DInput / 3DExtras）

## 构建

### 使用 Qt Creator（推荐）

1. 安装 Qt（在线安装器需勾选上述模块，以及 MinGW 64-bit 编译器）
2. 用 Qt Creator 打开工程文件 `untitled10.pro`
3. 选择构建套件（如 `Desktop Qt 5.15.2 MinGW 64-bit`）
4. 点击构建并运行

### 命令行（Windows / MinGW）

```bash
qmake untitled10.pro
mingw32-make
```

生成的可执行文件为 `FlightControlPC.exe`（Debug 输出位于 `build/` 下对应目录）。

## 目录结构

所有源码位于仓库根目录：

```
main.cpp                      # 程序入口
mainwindow.{h,cpp,ui}         # 主窗口（四个页面容器）
SerialTab.{h,cpp,ui}          # 串口通信页
serialthread.{h,cpp}          # 串口线程
bluetooththread.{h,cpp}       # 蓝牙通信
FlightStatusTab.{h,cpp,ui}    # 飞行状态页
FlyCtrlSetTab.{h,cpp,ui}      # 飞行控制设置页
calibratetab.{h,cpp}          # 传感器校准页
AttitudeDialWidget.{h,cpp}    # 姿态仪表盘
Drone3DView.{h,cpp}           # 3D 姿态视图
attitudewavechart.{h,cpp}     # 姿态波形图
GpsInfoPanel / ReceiverPanel / SensorDataPanel / FlightStatusPanel
untitled10.pro                # qmake 工程文件
```

## 说明

本项目为课程设计作品，仅供学习交流。
