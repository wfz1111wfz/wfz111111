#ifndef SERIALTHREAD_H
#define SERIALTHREAD_H
#include <cstdint>
#include <QByteArray>
#include <QString>
#include <QObject>
#include <QSerialPort>
#include <QEventLoop>
#include <QTimer>
#include <QVector>
#include <QDebug>

// ====================== 必须的帧头宏定义======================

#define ANO_HEAD3       0xAA
#define ANO_HEAD4       0xAF
#define BufferSizeMax   1024

// 串口配置结构体
typedef struct {
    QString serialPortNum;//串口号
    qint32 serialPortBaud;//波特率
    int serialPortCheckBit;//校验位
    int serialPortDataBit;//数据位
    int serialPortStopBit;//停止位
    QString serialPortDescription;
} SerialConfig;

class SerialThread : public QObject
{
    Q_OBJECT
public:
    explicit SerialThread(QObject *parent = nullptr);
    // 析构函数声明
    ~SerialThread();
    //发送函数
    void sendData(const QByteArray &data);
    void sendData(const char *data, qint64 len);

    // 兼容CalibrateTab添加的函数
    bool isOpen();
    void closeSerial();

private:
    QSerialPort *m_port;//真正操作串口硬件的指针
    void dataParse(uint8_t data);//协议解析函数
    QByteArray serialPortRead();//内部读取函数
    void serialPortWrite(const char *data, qint64 len);//内部写入函数

signals:

    void sig_openPortSuccess(bool isSuccess);//串口打开情况
    void sig_sendSerialPortProcessInfo(QString info, int color);//状态日志
    void sig_sendPortReadDataInfo(QByteArray buffer);//接收到飞控数据通知
    void sig_sendPortWriteDataInfo(QByteArray buffer);//发数据给飞控通知
    void sig_sendDebugParameterInfo(QString str, int color);
    void sig_sendCalibrationInfo(QString info, int color);//校准信息通知

    // sig_dataSort信号
    void sig_dataSort(QByteArray rxFrame);//收到完整帧
    void sig_closed();

public slots:

    void slot_openSerial();
    void slot_close_serial();
    void solt_doDataReceiveWork();//读取并解析函数
    void Delay_MSec(int msec);//延时


};

#endif // SERIALTHREAD_H