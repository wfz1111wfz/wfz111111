#include "SerialThread.h"
#include <cstring>

// ====================== 全局变量======================
long alSendNum = 0;
long alRecNum = 0;
long errorNum = 0;
bool isAppending = false;
uint8_t FREAM_HEAD = 0x00;
uint8_t CHECK_SUM = 0x00;
bool showProtocolFlag = true;
SerialConfig serialConfig = {
    "COM1", 115200, 0, 0, 0, ""
};
int delay_PID = 500;
bool enableBtn_PID = true;
bool enableBtn_CAL = true;

// ====================== 构造/析构 ======================
SerialThread::SerialThread(QObject *parent) : QObject(parent)
{
    m_port = new QSerialPort(this);//创建串口对象
    connect(m_port, &QSerialPort::readyRead, this, &SerialThread::solt_doDataReceiveWork);
}//当串口读到有数据时，执行读取解析操作

// 析构函数实现
SerialThread::~SerialThread()
{
    if (m_port && m_port->isOpen()) {
        m_port->close();
    }
}

// ====================== 发送数据 ======================
void SerialThread::sendData(const QByteArray &data)
{
    serialPortWrite(data.constData(), data.size());
}

void SerialThread::sendData(const char *data, qint64 len)
{
    serialPortWrite(data, len);
}

// ====================== 接口函数实现 ======================
bool SerialThread::isOpen()
{
    return m_port ? m_port->isOpen() : false;
}

void SerialThread::closeSerial()
{
    slot_close_serial();
    emit sig_closed();
}

// ====================== 串口读写 ======================
QByteArray SerialThread::serialPortRead()
{
    if (!m_port) return QByteArray();
    QByteArray buffer = m_port->readAll();//把数据全部读出来
    alRecNum += buffer.size();//字节数
    if (showProtocolFlag)
        emit sig_sendPortReadDataInfo(buffer);//数据发给界面显示
    return buffer;
}

void SerialThread::serialPortWrite(const char *data, qint64 len)
{
    if (!m_port || !m_port->isOpen()) return;
    QByteArray buffer(data, len);
    alSendNum += m_port->write(data, len);//把数据写入硬件，最终的发送
    emit sig_sendPortWriteDataInfo(buffer);
}

// ====================== 串口打开/关闭======================
void SerialThread::slot_openSerial()
{
    int infoSum = 1;
    emit sig_sendSerialPortProcessInfo("开始读取" + serialConfig.serialPortNum + "的参数列表...", 0);

    if (m_port->isOpen())
    {
        m_port->clear();
        m_port->close();
    }

    m_port->setPortName(serialConfig.serialPortNum);//读取串口号设置给QT底层
    infoSum += m_port->setBaudRate(serialConfig.serialPortBaud);//读取波特率设置给QT底层

    switch (serialConfig.serialPortCheckBit)
    {
    case 0: infoSum += m_port->setParity(QSerialPort::NoParity); break;
    case 1: infoSum += m_port->setParity(QSerialPort::OddParity); break;
    case 2: infoSum += m_port->setParity(QSerialPort::EvenParity); break;
    default: infoSum += m_port->setParity(QSerialPort::NoParity); break;
    }

    switch (serialConfig.serialPortDataBit)
    {
    case 0: infoSum += m_port->setDataBits(QSerialPort::Data8); break;
    case 1: infoSum += m_port->setDataBits(QSerialPort::Data7); break;
    case 2: infoSum += m_port->setDataBits(QSerialPort::Data6); break;
    case 3: infoSum += m_port->setDataBits(QSerialPort::Data5); break;
    default: infoSum += m_port->setDataBits(QSerialPort::Data8); break;
    }

    switch (serialConfig.serialPortStopBit)
    {
    case 0: infoSum += m_port->setStopBits(QSerialPort::OneStop); break;
    case 1: infoSum += m_port->setStopBits(QSerialPort::OneAndHalfStop); break;
    case 2: infoSum += m_port->setStopBits(QSerialPort::TwoStop); break;
    default: infoSum += m_port->setStopBits(QSerialPort::OneStop); break;
    }

    m_port->setFlowControl(QSerialPort::NoFlowControl);
    m_port->setReadBufferSize(BufferSizeMax);

    emit sig_sendSerialPortProcessInfo("参数列表读取成功!参数数量：" + QString::number(infoSum), 1);
    emit sig_sendSerialPortProcessInfo("开始连接串口...", 0);

    bool isSuccess = m_port->open(QIODevice::ReadWrite);//QT打开电脑串口和波特率
    emit sig_openPortSuccess(isSuccess);//发给界面结果

    if (isSuccess)
        emit sig_sendSerialPortProcessInfo("连接打开成功!自动读取设备信息", 2);
    else
    {
        emit sig_sendSerialPortProcessInfo("连接打开失败，请检查重试", 3);
        return;
    }
    emit sig_sendSerialPortProcessInfo("找到设备!" + serialConfig.serialPortDescription, 1);
}

void SerialThread::slot_close_serial()
{
    if (m_port && m_port->isOpen()) {
        m_port->close();
        emit sig_sendSerialPortProcessInfo("串口关闭成功!", 4);
    }
}

// ====================== 协议解析======================
void SerialThread::solt_doDataReceiveWork()
{
    QByteArray buffer = serialPortRead();//把缓冲区数据读出来
    for (int i = 0; i < buffer.size(); i++)
        dataParse(static_cast<uint8_t>(buffer.at(i)));
}

void SerialThread::dataParse(uint8_t data)
{
    static uint8_t RxBuffer[BufferSizeMax] = {0};
    static uint8_t _data_len = 0, _data_cnt = 0;
    static uint8_t state = 0;

    // 状态0：等待帧头1
    if (state == 0)
    {
        if (data == ANO_HEAD3)
        {
            memset(RxBuffer, 0, BufferSizeMax);
            state = 1;
            RxBuffer[0] = data;
        }
        return;
    }

    // 状态1：等待帧头2
    if (state == 1)
    {
        if ((RxBuffer[0] == ANO_HEAD3 && data == ANO_HEAD3))
        {
            state = 2;
            RxBuffer[1] = data;
        }
        else
        {
            state = 0;
        }
        return;
    }

    // 状态2：接收功能字
    if (state == 2)
    {
        state = 3;
        RxBuffer[2] = data;
        return;
    }

    // 状态3：接收数据长度
    if (state == 3)
    {
        state = 4;
        RxBuffer[3] = data;
        _data_len = data;
        _data_cnt = 0;
        if (_data_len == 0)
        {
            state = 5;
        }
        return;
    }

    // 状态4：接收数据段
    if (state == 4)
    {
        RxBuffer[4 + _data_cnt] = data;
        _data_cnt++;
        if (_data_cnt >= _data_len)
        {
            state = 5;
        }
        return;
    }

    // 状态5：接收校验和，验证并处理
    if (state == 5)
    {
        RxBuffer[4 + _data_cnt] = data;
        uint8_t frame_total_len = 4 + _data_cnt + 1;

        // 校验和计算
        uint8_t sum = 0;
        for (uint8_t i = 0; i < frame_total_len - 1; i++)
        {
            sum += RxBuffer[i];
        }

        // 校验和验证
        if (sum == data)
        {
            QByteArray frame((char*)RxBuffer, frame_total_len);
            // 只要是AA AA的帧，强制发送信号
            if (RxBuffer[0] == ANO_HEAD3 && RxBuffer[1] == ANO_HEAD3)
            {
                emit sig_dataSort(frame);
            }


        }
        else
        {
            errorNum++;
        }

        // 重置状态机
        state = 0;
        _data_len = 0;
        _data_cnt = 0;
        memset(RxBuffer, 0, BufferSizeMax);
        return;
    }

    // 异常重置
    state = 0;
    _data_len = 0;
    _data_cnt = 0;
    memset(RxBuffer, 0, BufferSizeMax);
}

// ====================== 延时函数 ======================
void SerialThread::Delay_MSec(int msec)
{
    QEventLoop loop;
    QTimer::singleShot(msec, &loop, &QEventLoop::quit);
    loop.exec();
}




