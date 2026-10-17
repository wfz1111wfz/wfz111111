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
