#include "SerialTab.h"
#include "ui_SerialTab.h"
#include <QSerialPortInfo>
#include <QMessageBox>
#include <QDebug>

extern SerialConfig serialConfig;

SerialTab::SerialTab(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SerialTab)
{
    ui->setupUi(this);//创建QT界面

     //初始化波特率下拉框
    ui->comBaud->clear(); // 先清空原来的旧选项
    // 添加所有波特率
    ui->comBaud->addItem("1200", 1200);
    ui->comBaud->addItem("4800", 4800);
    ui->comBaud->addItem("9600", 9600);
    ui->comBaud->addItem("38400", 38400);
    ui->comBaud->addItem("115200", 115200);
    ui->comBaud->addItem("256000", 256000);
    ui->comBaud->addItem("500000", 500000);
    // 默认选中最常用的 115200
    ui->comBaud->setCurrentText("115200");

    // ========== 串口初始化 ==========
    m_serialThread = new SerialThread(this);//创建串口进行收发数据
    initSerialPortList();//扫描电脑串口显示到下拉框
    //串口接收到消息显示到接收框
    connect(m_serialThread, &SerialThread::sig_sendSerialPortProcessInfo, this, [=](QString str, int color) {
        Q_UNUSED(color);//状态日志显示文字
        ui->txtRecv->append("[串口] " + str);
    });
    // 串口数据显示：仅串口打开时显示
    connect(m_serialThread, &SerialThread::sig_sendPortReadDataInfo, this, [=](QByteArray data) {
        if(!m_serialThread->isOpen()) return;
        if (ui->chkHexRecv->isChecked()) {
            ui->txtRecv->append("[串口] " + data.toHex(' ').toUpper());//显示十六进制，大写
        } else {
            ui->txtRecv->append("[串口] " + QString::fromUtf8(data));//显示字符串
        }
    });
    connect(m_serialThread, &SerialThread::sig_openPortSuccess, this, [=](bool isSuccess) {
        updateSerialUIState(isSuccess);
        ui->txtRecv->append(isSuccess ? "[通信] 连接成功！" : "[通信] 连接失败！");
    });//点击连接，刷新状态判断有没有连接成功
    connect(m_serialThread, &SerialThread::sig_closed, this, [=]() {
        updateSerialUIState(false);
        ui->txtRecv->append("[通信] 已断开");
    });

    // ========== BLE蓝牙初始化 ==========
    m_bluetoothThread = new BluetoothThread(this);

    connect(m_bluetoothThread, &BluetoothThread::sig_bleDeviceFound,
            this, &SerialTab::onBleDeviceFound);//搜索蓝牙设备，并添加到下拉框
    connect(m_bluetoothThread, &BluetoothThread::sig_bleDiscoveryFinished,
            this, [=](){
                ui->btnRefresh_Bt->setEnabled(true);
                ui->btnRefresh_Bt->setText("扫描蓝牙设备");
                ui->txtRecv->append("[蓝牙] 扫描完成，共找到 " + QString::number(ui->comBtDevice->count()) + " 个设备");
            });//扫描结束后打印搜索到的设备
    connect(m_bluetoothThread, &BluetoothThread::sig_connected,
            this, [=](){
                updateBluetoothUIState(true);
                ui->txtRecv->append("[蓝牙] 连接成功！");
            });
    connect(m_bluetoothThread, &BluetoothThread::sig_disconnected,
            this, [=](){
                updateBluetoothUIState(false);
                ui->txtRecv->append("[蓝牙] 已断开");
            });
    connect(m_bluetoothThread, &BluetoothThread::sig_bleError,
            this, [=](QString msg){
                ui->txtRecv->append("[错误] " + msg);
            });

    // 蓝牙数据解析，传到serialthread
    connect(m_bluetoothThread, &BluetoothThread::sig_dataSort,
            m_serialThread, &SerialThread::solt_doDataReceiveWork);
    // 蓝牙数据显示：仅蓝牙连接时显示
    connect(m_bluetoothThread, &BluetoothThread::sig_dataSort, this, &SerialTab::onRecvData_BLE);

    // 自动连接飞控
    connect(ui->comBtDevice, QOverload<int>::of(&QComboBox::activated), this, [=](int index){
        Q_UNUSED(index);
        QString currentText = ui->comBtDevice->currentText();
        if (currentText.contains("GDUT_UAV") || currentText.contains("2A:C0:19:01:01:04")) {
            on_btnConnect_Bt_clicked();
        }
    });

    updateSerialUIState(false);
    updateBluetoothUIState(false);
}

// 蓝牙数据显示函数
void SerialTab::onRecvData_BLE(QByteArray data)
{
    if(!m_bluetoothThread->isConnected()) return;//判断连接状态
    if (ui->chkHexRecv->isChecked()) {
        ui->txtRecv->append("[蓝牙] " + data.toHex(' ').toUpper());
    } else {
        ui->txtRecv->append("[蓝牙] " + QString::fromUtf8(data));
    }//判断是八进制还是十六进制
}
//关闭窗口执行
SerialTab::~SerialTab()
{
    m_serialThread->closeSerial();
    m_bluetoothThread->disconnectDevice();
    delete ui;
}

// ====================== 串口 ======================
void SerialTab::initSerialPortList()
{
    ui->comCom->clear();//清空com
    const auto infos = QSerialPortInfo::availablePorts();//重新获取电脑串口
    for (const QSerialPortInfo &info : infos) {
        ui->comCom->addItem(info.portName(), info.portName());//重新将串口添加到下拉框
    }
}
//更新串口按钮状态
void SerialTab::updateSerialUIState(bool isOpened)
{
    ui->btnOpen_Serial->setEnabled(!isOpened);
    ui->btnClose_Serial->setEnabled(isOpened);
    ui->btnRefresh_Serial->setEnabled(!isOpened);
    ui->comCom->setEnabled(!isOpened);
    ui->comBaud->setEnabled(!isOpened);
}

void SerialTab::on_btnRefresh_Serial_clicked()
{
    initSerialPortList();
    ui->txtRecv->append("[系统] 已刷新串口");
}

void SerialTab::on_btnOpen_Serial_clicked()
{
    if (ui->comCom->currentText().isEmpty()) {
        QMessageBox::warning(this, "提示", "请选串口");
        return;
    }
    serialConfig.serialPortNum = ui->comCom->currentText();
    serialConfig.serialPortBaud = ui->comBaud->currentText().toInt();
    m_serialThread->slot_openSerial();//调用打开串口函数
}

void SerialTab::on_btnClose_Serial_clicked()
{
    m_serialThread->closeSerial();
}

// ====================== BLE蓝牙 ======================
void SerialTab::on_btnRefresh_Bt_clicked()
{
    ui->comBtDevice->clear();
    ui->txtRecv->append("[蓝牙] 开始扫描 BLE (30秒超时)...");
    ui->btnRefresh_Bt->setEnabled(false);
    ui->btnRefresh_Bt->setText("扫描中...");
    m_bluetoothThread->startBleDiscovery();//扫描蓝牙函数，调用扫描函数
}

void SerialTab::onBleDeviceFound(const QBluetoothDeviceInfo &deviceInfo)
{
    QString mac = deviceInfo.address().toString().toUpper();
    QString name = deviceInfo.name();
    if (name.isEmpty()) name = "未知设备";
//
    QString show = QString("%1 | %2").arg(name).arg(mac);
    ui->comBtDevice->addItem(show, QVariant::fromValue(deviceInfo));
    ui->txtRecv->append("[发现] " + show);//把设备和mac地址都显示在界面

    if (mac == "2A:C0:19:01:01:04") {
        ui->comBtDevice->setCurrentIndex(ui->comBtDevice->count()-1);
        ui->txtRecv->append("[蓝牙] ✅ 自动定位到 GDUT_UAV 飞控！");
    }
}
//连接蓝牙设备
void SerialTab::on_btnConnect_Bt_clicked()
{
    if (ui->comBtDevice->currentData().isNull()) {
        QMessageBox::warning(this, "提示", "请先扫描并选择设备");
        return;
    }
    QBluetoothDeviceInfo dev = ui->comBtDevice->currentData().value<QBluetoothDeviceInfo>();
    ui->txtRecv->append("[蓝牙] 正在连接: " + ui->comBtDevice->currentText());
    m_bluetoothThread->connectToDevice(dev);//调用连接函数
}
//断开蓝牙设备
void SerialTab::on_btnDisconnect_Bt_clicked()
{
    m_bluetoothThread->disconnectDevice();
}
//刷新状态
void SerialTab::updateBluetoothUIState(bool isConnected)
{
    ui->btnConnect_Bt->setEnabled(!isConnected);
    ui->btnDisconnect_Bt->setEnabled(isConnected);
    ui->btnRefresh_Bt->setEnabled(!isConnected);
    ui->comBtDevice->setEnabled(!isConnected);
}

// ====================== 通用收发 ======================
void SerialTab::on_btnClearRecv_clicked()
{
    ui->txtRecv->clear();
}

void SerialTab::on_btnClearSend_clicked()
{
    ui->txtSend->clear();
}

void SerialTab::on_btnSend_clicked()
{
    QString text = ui->txtSend->toPlainText();
    if (text.isEmpty()) return;

    QByteArray data;
    if (ui->chkHexSend->isChecked()) {
        data = QByteArray::fromHex(text.toUtf8());//字节转换
    } else {
        data = text.toUtf8();
    }
//判断发送
    if (m_serialThread->isOpen()) m_serialThread->sendData(data);
    if (m_bluetoothThread->isConnected()) m_bluetoothThread->sendData(data);
}