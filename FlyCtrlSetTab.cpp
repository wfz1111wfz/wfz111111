#include "FlyCtrlSetTab.h"
#include "ui_FlyCtrlSetTab.h"
#include "BluetoothThread.h" // 包含蓝牙线程头文件
#include <QTime>
#include <QMessageBox>
#include <QDebug>

// ====================== 构造函数 ======================
// 构造函数现在接收两个参数：串口线程 + 蓝牙线程
FlyCtrlSetTab::FlyCtrlSetTab(SerialThread *serialThread, BluetoothThread *bluetoothThread, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FlyCtrlSetTab)
    , m_serialThread(serialThread)      // 保存串口线程指针
    , m_bluetoothThread(bluetoothThread) // 保存蓝牙线程指针
{
    ui->setupUi(this);
    initUI(); // 初始化界面上的18组PID输入框

    // 初始化PID写入状态机变量
    m_currentWriteFrameIndex = 0; // 当前正在写入第几帧（范围0-5，共6帧）
    m_writeRetryCount = 0;         // 当前帧的重发次数
    m_lastFrameCheckSum = 0;       // 记录上一帧的帧尾校验和
    m_lastPayloadCheckSum = 0;     // 记录上一帧的Payload校验和（用于比对飞控应答）

    // 初始化超时定时器
    m_writeTimeoutTimer = new QTimer(this);
    m_writeTimeoutTimer->setSingleShot(true); // 设置为单次触发模式
    m_writeTimeoutTimer->setInterval(TIMEOUT_MS); // 超时时间1000ms
    // 连接超时信号：如果1秒内没收到飞控应答，就触发 onWritePidTimeout 进行重发
    connect(m_writeTimeoutTimer, &QTimer::timeout, this, &FlyCtrlSetTab::onWritePidTimeout);

    // 检查有无连接
    // 只要线程解析出完整帧，就通过 sig_dataSort 发到这里的 parsePIDData 进行处理
    if (m_serialThread) {
        connect(m_serialThread, &SerialThread::sig_dataSort, this, &FlyCtrlSetTab::parsePIDData, Qt::QueuedConnection);
    }
    if (m_bluetoothThread) {
        connect(m_bluetoothThread, &BluetoothThread::sig_dataSort, this, &FlyCtrlSetTab::parsePIDData, Qt::QueuedConnection);
    }
}

// ====================== 析构函数 ======================
FlyCtrlSetTab::~FlyCtrlSetTab()
{
    m_writeTimeoutTimer->stop(); // 停止定时器
    delete m_writeTimeoutTimer;   // 释放定时器
    delete ui;                    // 释放UI
}

// ====================== 动态设置串口线程======================
void FlyCtrlSetTab::setSerialThread(SerialThread* thread)
{
    if(!thread) return;
    // 如果之前有连接，先断开旧的信号槽
    if(m_serialThread) disconnect(m_serialThread, nullptr, this, nullptr);

    m_serialThread = thread;
    // 连接新的串口数据信号
    connect(m_serialThread, &SerialThread::sig_dataSort, this, &FlyCtrlSetTab::parsePIDData, Qt::QueuedConnection);
}

// ====================== 判断连接状态 ======================
//判断「串口或蓝牙」是否有任意一个连接成功
bool FlyCtrlSetTab::isConnected()
{
    bool serialOk = (m_serialThread && m_serialThread->isOpen());
    bool bleOk    = (m_bluetoothThread && m_bluetoothThread->isConnected());
    return serialOk || bleOk;
}

// ====================== 统一数据发送接口 ======================
void FlyCtrlSetTab::sendData(const QByteArray &data)
{
    // 如果串口打开了，优先/同时通过串口发送
    if (m_serialThread && m_serialThread->isOpen()) {
        m_serialThread->sendData(data);
    }
    // 如果蓝牙连接了，同时通过蓝牙发送
    if (m_bluetoothThread && m_bluetoothThread->isConnected()) {
        m_bluetoothThread->sendData(data);
    }
}

// ====================== 初始化UI：设置输入框范围 ======================
void FlyCtrlSetTab::initUI()
{
    // 定义18个PID输入框的指针数组
    QSpinBox *pBoxes[18] = {
        ui->sb_p1, ui->sb_p2, ui->sb_p3, ui->sb_p4, ui->sb_p5, ui->sb_p6,
        ui->sb_p7, ui->sb_p8, ui->sb_p9, ui->sb_p10, ui->sb_p11, ui->sb_p12,
        ui->sb_p13, ui->sb_p14, ui->sb_p15, ui->sb_p16, ui->sb_p17, ui->sb_p18
    };
    QSpinBox *iBoxes[18] = {
        ui->sb_i1, ui->sb_i2, ui->sb_i3, ui->sb_i4, ui->sb_i5, ui->sb_i6,
        ui->sb_i7, ui->sb_i8, ui->sb_i9, ui->sb_i10, ui->sb_i11, ui->sb_i12,
        ui->sb_i13, ui->sb_i14, ui->sb_i15, ui->sb_i16, ui->sb_i17, ui->sb_i18
    };
    QSpinBox *dBoxes[18] = {
        ui->sb_d1, ui->sb_d2, ui->sb_d3, ui->sb_d4, ui->sb_d5, ui->sb_d6,
        ui->sb_d7, ui->sb_d8, ui->sb_d9, ui->sb_d10, ui->sb_d11, ui->sb_d12,
        ui->sb_d13, ui->sb_d14, ui->sb_d15, ui->sb_d16, ui->sb_d17, ui->sb_d18
    };

    // 循环设置所有输入框的范围为 -32768 ~ +32767
    for(int i=0; i<18; i++) {
        pBoxes[i]->setRange(-32768, 32767);
        iBoxes[i]->setRange(-32768, 32767);
        dBoxes[i]->setRange(-32768, 32767);
    }
}

// ====================== 收集界面数据：把18组PID读到内存里 ======================
void FlyCtrlSetTab::collectAllPIDData()
{
    m_allKp.clear(); m_allKi.clear(); m_allKd.clear();
    // 读取所有 P 值
    m_allKp << (int16_t)ui->sb_p1->value() << (int16_t)ui->sb_p2->value() << (int16_t)ui->sb_p3->value()
            << (int16_t)ui->sb_p4->value() << (int16_t)ui->sb_p5->value() << (int16_t)ui->sb_p6->value()
            << (int16_t)ui->sb_p7->value() << (int16_t)ui->sb_p8->value() << (int16_t)ui->sb_p9->value()
            << (int16_t)ui->sb_p10->value() << (int16_t)ui->sb_p11->value() << (int16_t)ui->sb_p12->value()
            << (int16_t)ui->sb_p13->value() << (int16_t)ui->sb_p14->value() << (int16_t)ui->sb_p15->value()
            << (int16_t)ui->sb_p16->value() << (int16_t)ui->sb_p17->value() << (int16_t)ui->sb_p18->value();
    // 读取所有 I 值
    m_allKi << (int16_t)ui->sb_i1->value() << (int16_t)ui->sb_i2->value() << (int16_t)ui->sb_i3->value()
            << (int16_t)ui->sb_i4->value() << (int16_t)ui->sb_i5->value() << (int16_t)ui->sb_i6->value()
            << (int16_t)ui->sb_i7->value() << (int16_t)ui->sb_i8->value() << (int16_t)ui->sb_i9->value()
            << (int16_t)ui->sb_i10->value() << (int16_t)ui->sb_i11->value() << (int16_t)ui->sb_i12->value()
            << (int16_t)ui->sb_i13->value() << (int16_t)ui->sb_i14->value() << (int16_t)ui->sb_i15->value()
            << (int16_t)ui->sb_i16->value() << (int16_t)ui->sb_i17->value() << (int16_t)ui->sb_i18->value();
    // 读取所有 D 值
    m_allKd << (int16_t)ui->sb_d1->value() << (int16_t)ui->sb_d2->value() << (int16_t)ui->sb_d3->value()
            << (int16_t)ui->sb_d4->value() << (int16_t)ui->sb_d5->value() << (int16_t)ui->sb_d6->value()
            << (int16_t)ui->sb_d7->value() << (int16_t)ui->sb_d8->value() << (int16_t)ui->sb_d9->value()
            << (int16_t)ui->sb_d10->value() << (int16_t)ui->sb_d11->value() << (int16_t)ui->sb_d12->value()
            << (int16_t)ui->sb_d13->value() << (int16_t)ui->sb_d14->value() << (int16_t)ui->sb_d15->value()
            << (int16_t)ui->sb_d16->value() << (int16_t)ui->sb_d17->value() << (int16_t)ui->sb_d18->value();
}

// ====================== UI 按钮：清空日志 ======================
void FlyCtrlSetTab::on_tbtn_clear_clicked()
{
    ui->tb_showMsg->clear();
    showMsg("日志已清空");
}

// ====================== UI 按钮：清空所有PID输入框 ======================
void FlyCtrlSetTab::on_btn_clearSpinBox_clicked()
{
    QSpinBox *pBoxes[18] = {
        ui->sb_p1, ui->sb_p2, ui->sb_p3, ui->sb_p4, ui->sb_p5, ui->sb_p6,
        ui->sb_p7, ui->sb_p8, ui->sb_p9, ui->sb_p10, ui->sb_p11, ui->sb_p12,
        ui->sb_p13, ui->sb_p14, ui->sb_p15, ui->sb_p16, ui->sb_p17, ui->sb_p18
    };
    QSpinBox *iBoxes[18] = {
        ui->sb_i1, ui->sb_i2, ui->sb_i3, ui->sb_i4, ui->sb_i5, ui->sb_i6,
        ui->sb_i7, ui->sb_i8, ui->sb_i9, ui->sb_i10, ui->sb_i11, ui->sb_i12,
        ui->sb_i13, ui->sb_i14, ui->sb_i15, ui->sb_i16, ui->sb_i17, ui->sb_i18
    };
    QSpinBox *dBoxes[18] = {
        ui->sb_d1, ui->sb_d2, ui->sb_d3, ui->sb_d4, ui->sb_d5, ui->sb_d6,
        ui->sb_d7, ui->sb_d8, ui->sb_d9, ui->sb_d10, ui->sb_d11, ui->sb_d12,
        ui->sb_d13, ui->sb_d14, ui->sb_d15, ui->sb_d16, ui->sb_d17, ui->sb_d18
    };
    // 循环清零
    for(int i=0; i<18; i++) {
        pBoxes[i]->setValue(0);
        iBoxes[i]->setValue(0);
        dBoxes[i]->setValue(0);
    }
    showMsg("所有PID已清零");
}

// ====================== UI 按钮：点击写入PID ======================
void FlyCtrlSetTab::on_btn_writePID_clicked()
{
    // 连接判断函数
    if (!isConnected()) {
        QMessageBox::warning(this, "错误", "未连接设备！");
        return;
    }

    // 重置状态机，准备开始写入
    m_writeTimeoutTimer->stop();
    m_currentWriteFrameIndex = 0; // 从第0帧开始
    m_writeRetryCount = 0;
    collectAllPIDData(); // 先把界面上的数值读进数组

    // 发送第一帧
    sendSinglePidFrame(m_currentWriteFrameIndex);
    showMsg("开始写入PID → 共6帧");
}

// ====================== UI 按钮：点击读取PID ======================
void FlyCtrlSetTab::on_btn_readPID_clicked()
{
    // 检查连接
    if (!isConnected()) {
        QMessageBox::warning(this, "错误", "未连接设备！");
        return;
    }

    // 组装读取指令帧
    QByteArray cmd;
    cmd.append(TX_HEAD1);
    cmd.append(TX_HEAD2);
    cmd.append(FUNC_CMD_SET2);
    cmd.append((uint8_t)0x01);
    cmd.append(FUNC_READ_PID); // 0x01

    // 计算并追加校验和
    uint8_t check = calcChecksum((uint8_t*)cmd.constData(), cmd.size());
    cmd.append(check);

    sendData(cmd);
    showMsg("已发送读取PID指令");
}

// ====================== UI 按钮：点击恢复默认PID ======================
void FlyCtrlSetTab::on_btn_recoverDefault_clicked()
{
    // 检查连接
    if (!isConnected()) {
        QMessageBox::warning(this, "错误", "未连接设备！");
        return;
    }

    // 组装恢复默认指令帧
    QByteArray cmd;
    cmd.append(TX_HEAD1);
    cmd.append(TX_HEAD2);
    cmd.append(FUNC_CMD_SET2);
    cmd.append((uint8_t)0x01);
    cmd.append(CMD2_RESTORE_DEFAULT);

    // 计算并追加校验和
    uint8_t check = calcChecksum((uint8_t*)cmd.constData(), cmd.size());
    cmd.append(check);


    sendData(cmd);
    showMsg("✅ 已发送恢复默认参数指令");
}

// ====================== UI 按钮：点击保存PID ======================
void FlyCtrlSetTab::on_btn_savePID_clicked()
{
    // 检查连接
    if (!isConnected()) {
        QMessageBox::warning(this, "错误", "未连接设备！");
        return;
    }

    // 组装保存指令帧
    QByteArray cmd;
    cmd.append(TX_HEAD1);
    cmd.append(TX_HEAD2);
    cmd.append(FUNC_SAVE_PID);
    cmd.append((uint8_t)0x00);

    // 计算并追加校验和
    uint8_t check = calcChecksum((uint8_t*)cmd.constData(), cmd.size());
    cmd.append(check);

    sendData(cmd);
    showMsg("✅ 保存PID指令已发送");
}

// ====================== 超时处理：重发逻辑 ======================
void FlyCtrlSetTab::onWritePidTimeout()
{
    m_writeRetryCount++;
    // 超过最大重发次数（5次），放弃并报错
    if (m_writeRetryCount > MAX_RETRY_COUNT) {
        showMsg("❌ 写入PID超时，重发失败");
        m_currentWriteFrameIndex = 0;
        m_writeRetryCount = 0;
        return;
    }
    // 没超过次数，重新发送当前帧
    sendSinglePidFrame(m_currentWriteFrameIndex);
    showMsg(QString("⚠️ 第%1次重发第%2帧").arg(m_writeRetryCount).arg(m_currentWriteFrameIndex+1));
}

// ======================处理飞控发回来的数据 ======================
void FlyCtrlSetTab::parsePIDData(QByteArray rxFrame)
{
    if (rxFrame.size() < 6) return; // 帧长不够，丢弃

    uint8_t h1 = (uint8_t)rxFrame[0];
    uint8_t h2 = (uint8_t)rxFrame[1]; // 判断帧头
    if (h1 != RX_HEAD1 || h2 != RX_HEAD2) return; // 帧头不是 0xAAAA，丢弃

    uint8_t func    = (uint8_t)rxFrame[2]; // 判断功能字
    uint8_t payLen  = (uint8_t)rxFrame[3]; // 判断数据长度
    int totalLen    = 4 + payLen + 1;       // 计算总长度

    if (rxFrame.size() != totalLen) return; // 实际长度与计算长度不符，丢弃

    // 全帧累加校验，确保帧本身没传错
    uint8_t sum = 0;
    for (int i=0; i<totalLen-1; i++) sum += (uint8_t)rxFrame[i];
    if (sum != (uint8_t)rxFrame[totalLen-1]) return; // 校验和错误，丢弃

    // 分支1：读取PID回传 (功能字 0x10~0x15)
    if (func >= FUNC_WRITE_PID_BASE && func <= FUNC_WRITE_PID_BASE +5)
    {
        parseSinglePidFrame((uint8_t*)rxFrame.constData(), rxFrame.size()); // 解析PID数值
        int n = func - FUNC_WRITE_PID_BASE;
        showMsg(QString("✅ 读取PID组%1成功").arg(n+1));
        return;
    }

    // 分支2：CHECK应答帧 (功能字 0xEF)
    if (func == FUNC_ACK_CHECK && payLen == CHECK_FRAME_PAYLOAD_LEN)
    {
        uint8_t ackFunc = (uint8_t)rxFrame[4];  // 飞控应答的是哪个功能字
        uint8_t ackCheck = (uint8_t)rxFrame[5]; // 飞控发回来的Payload校验和

        qDebug() << "[CHECK] 应答功能字:" << QString::number(ackFunc, 16).toUpper()
                 << "应答校验和:" << QString::number(ackCheck, 16).toUpper()
                 << "本地Payload校验和:" << QString::number(m_lastPayloadCheckSum, 16).toUpper();

        // 检查：应答的功能字是否等于刚才发送的功能字
        uint8_t targetWriteFunc = FUNC_WRITE_PID_BASE + m_currentWriteFrameIndex;
        if (ackFunc == targetWriteFunc && m_currentWriteFrameIndex <6)
        {
            // 检查：飞控返回的校验和是否等于本地计算的校验和
            if (ackCheck == m_lastPayloadCheckSum) {
                m_writeTimeoutTimer->stop(); // 停止超时定时器
                showMsg(QString("✅ 第%1帧写入成功（校验和匹配）").arg(m_currentWriteFrameIndex+1));

                // 写下一帧
                m_currentWriteFrameIndex++;
                if (m_currentWriteFrameIndex <6) {
                    sendSinglePidFrame(m_currentWriteFrameIndex);
                } else {
                    showMsg("全部6帧PID写入完成！");
                    m_currentWriteFrameIndex =0;
                }
            } else {
                // 校验和比对失败，重发当前帧
                showMsg("❌ 校验和错误，重发当前帧");
                onWritePidTimeout();
            }
            return;
        }
    }
}

// ====================== 发送单帧PID数据 ======================
void FlyCtrlSetTab::sendSinglePidFrame(uint8_t frameIndex)
{
    if (frameIndex >=6 || m_allKp.size()<18) return; // 越界检查

    QByteArray frame;
    // 1. 填充帧头
    frame.append(TX_HEAD1);
    frame.append(TX_HEAD2);
    // 2. 填充功能字 (0x10, 0x11 ... 0x15)
    frame.append(FUNC_WRITE_PID_BASE + frameIndex);
    // 3. 填充数据长度 (固定18字节：3组PID * 6字节/组)
    frame.append(PAYLOAD_LEN_SINGLE_PID);

    // 4. 填充数据段：3组PID
    int start = frameIndex *3;
    for(int i=0; i<3; i++){
        int idx = start +i;
        uint16_t p = m_allKp[idx];
        uint16_t iVal = m_allKi[idx];
        uint16_t d = m_allKd[idx]; // 从pid表格中读出对应的pid值

        //高字节在前，低字节在后
        frame.append((p>>8)&0xFF);    // P高8位
        frame.append(p&0xFF);         // P低8位
        frame.append((iVal>>8)&0xFF); // I高8位
        frame.append(iVal&0xFF);      // I低8位
        frame.append((d>>8)&0xFF);    // D高8位
        frame.append(d&0xFF);         // D低8位
    }

    // 5. 计算校验和
    uint8_t frameCheckSum = calcChecksum((uint8_t*)frame.constData(), frame.size());
    uint8_t payloadCheckSum = calcChecksum((uint8_t*)frame.constData(), frame.size()-1);
    frame.append(frameCheckSum); // 追加到帧尾

    // 保存校验和，用于后续比对飞控的应答
    m_lastFrameCheckSum = frameCheckSum;
    m_lastPayloadCheckSum = payloadCheckSum;

    qDebug() << "[发送PID] 第" << frameIndex+1 << "帧:" << frame.toHex(' ').toUpper()
             << "帧尾校验和:" << QString::number(frameCheckSum, 16).toUpper()
             << "Payload校验和:" << QString::number(payloadCheckSum, 16).toUpper();

    // 使用统一发送接口发送数据
    sendData(frame);
    // 启动超时定时器，等待飞控应答
    m_writeTimeoutTimer->start();
}

// ====================== 解析PID回传数据并填到UI ======================
void FlyCtrlSetTab::parseSinglePidFrame(uint8_t *frameData, int frameLen)
{
    Q_UNUSED(frameLen);
    uint8_t func = frameData[2];
    int start = (func - FUNC_WRITE_PID_BASE)*3; // 判断是第几组pid
    int off =4;

    for(int i=0; i<3; i++){
        int idx = start +i;
        // 高字节在前，拼成一个16位的pid值
        uint16_t rawKp = (frameData[off]<<8) | frameData[off+1]; off+=2;
        uint16_t rawKi = (frameData[off]<<8) | frameData[off+1]; off+=2;
        uint16_t rawKd = (frameData[off]<<8) | frameData[off+1]; off+=2;
        int16_t kp = (int16_t)rawKp;
        int16_t ki = (int16_t)rawKi;
        int16_t kd = (int16_t)rawKd;
        fillPidToUI(idx, kp, ki, kd); // 填到界面上
    } // 循环解析三组pid
}

// ====================== 把PID数值填到具体的输入框 ======================
void FlyCtrlSetTab::fillPidToUI(int idx, int16_t kp, int16_t ki, int16_t kd)
{
    // 指针数组映射UI控件
    QSpinBox *p[] = {ui->sb_p1,ui->sb_p2,ui->sb_p3,ui->sb_p4,ui->sb_p5,ui->sb_p6,
                     ui->sb_p7,ui->sb_p8,ui->sb_p9,ui->sb_p10,ui->sb_p11,ui->sb_p12,
                     ui->sb_p13,ui->sb_p14,ui->sb_p15,ui->sb_p16,ui->sb_p17,ui->sb_p18};
    QSpinBox *i[] = {ui->sb_i1,ui->sb_i2,ui->sb_i3,ui->sb_i4,ui->sb_i5,ui->sb_i6,
                     ui->sb_i7,ui->sb_i8,ui->sb_i9,ui->sb_i10,ui->sb_i11,ui->sb_i12,
                     ui->sb_i13,ui->sb_i14,ui->sb_i15,ui->sb_i16,ui->sb_i17,ui->sb_i18};
    QSpinBox *d[] = {ui->sb_d1,ui->sb_d2,ui->sb_d3,ui->sb_d4,ui->sb_d5,ui->sb_d6,
                     ui->sb_d7,ui->sb_d8,ui->sb_d9,ui->sb_d10,ui->sb_d11,ui->sb_d12,
                     ui->sb_d13,ui->sb_d14,ui->sb_d15,ui->sb_d16,ui->sb_d17,ui->sb_d18};

    if(idx>=18) return;
    p[idx]->setValue(kp);
    i[idx]->setValue(ki);
    d[idx]->setValue(kd);
}

// ====================== 计算校验和（累加和） ======================
uint8_t FlyCtrlSetTab::calcChecksum(uint8_t *data, int len)
{
    uint8_t sum=0;
    for(int i=0; i<len; i++) sum+=data[i];
    return sum;
}

// ====================== 在界面日志框显示消息 ======================
void FlyCtrlSetTab::showMsg(const QString &msg)
{
    ui->tb_showMsg->append(QString("[%1] %2").arg(QTime::currentTime().toString("hh:mm:ss")).arg(msg));
}