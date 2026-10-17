#ifndef FLYCTRLSETTAB_H
#define FLYCTRLSETTAB_H

#include <QWidget>
#include <QVector>
#include <QTimer>
#include <QSpinBox>
#include "SerialThread.h"

// 蓝牙前向声明
class BluetoothThread;

namespace Ui {
class FlyCtrlSetTab;
}

class FlyCtrlSetTab : public QWidget
{
    Q_OBJECT

public:
    // 构造函数支持蓝牙
    explicit FlyCtrlSetTab(SerialThread *serialThread = nullptr, BluetoothThread *bluetoothThread = nullptr, QWidget *parent = nullptr);
    ~FlyCtrlSetTab();
    void setSerialThread(SerialThread* thread);

private slots:
    void on_tbtn_clear_clicked();
    void on_btn_clearSpinBox_clicked();
    void on_btn_writePID_clicked();
    void on_btn_readPID_clicked();
    void on_btn_recoverDefault_clicked();
    void on_btn_savePID_clicked();

    void onWritePidTimeout();
    void parsePIDData(QByteArray rxFrame);

private:
    Ui::FlyCtrlSetTab *ui;
    SerialThread  *m_serialThread;
    BluetoothThread *m_bluetoothThread;

    uint8_t  m_currentWriteFrameIndex;
    uint8_t  m_writeRetryCount;
    uint8_t  m_lastFrameCheckSum;
    uint8_t  m_lastPayloadCheckSum;
    QVector<int16_t> m_allKp, m_allKi, m_allKd;
    QTimer *m_writeTimeoutTimer;

    void initUI();
    void collectAllPIDData();
    void showMsg(const QString &msg);
    uint8_t calcChecksum(uint8_t *data, int len);
    void sendSinglePidFrame(uint8_t frameIndex);
    void parseSinglePidFrame(uint8_t *frameData, int frameLen);
    void fillPidToUI(int idx, int16_t kp, int16_t ki, int16_t kd);

    // 工具函数：判断是否连接（串口/蓝牙任意一个都行）
    bool isConnected();
    // 工具函数：自动用串口 or 蓝牙发数据
    void sendData(const QByteArray &data);

    static const uint8_t TX_HEAD1 = 0xAA;
    static const uint8_t TX_HEAD2 = 0xAF;
    static const uint8_t RX_HEAD1 = 0xAA;
    static const uint8_t RX_HEAD2 = 0xAA;

    static const uint8_t FUNC_READ_PID       = 0x01;
    static const uint8_t FUNC_CMD_SET2       = 0x02;
    static const uint8_t FUNC_SAVE_PID       = 0x04;
    static const uint8_t FUNC_WRITE_PID_BASE = 0x10;
    static const uint8_t FUNC_ACK_CHECK      = 0xEF;

    static const uint8_t CMD2_RESTORE_DEFAULT = 0xA1;
    static const uint8_t PAYLOAD_LEN_SINGLE_PID    = 18;
    static const uint8_t CHECK_FRAME_PAYLOAD_LEN  = 2;

    static const int MAX_RETRY_COUNT = 5;
    static const int TIMEOUT_MS = 1000;
};

#endif // FLYCTRLSETTAB_H