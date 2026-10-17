#ifndef SERIALTAB_H
#define SERIALTAB_H
#include <QWidget>
#include <QSerialPortInfo>
#include "SerialThread.h"
#include "BluetoothThread.h"
#include <QBluetoothDeviceInfo>

namespace Ui {
class SerialTab;
}

class SerialTab : public QWidget
{
    Q_OBJECT

public:
    explicit SerialTab(QWidget *parent = nullptr);
    ~SerialTab();

    SerialThread* getSerialThread() { return m_serialThread; }
    BluetoothThread* getBluetoothThread() { return m_bluetoothThread; }

    // 蓝牙数据显示槽函数
public slots:
    void onRecvData_BLE(QByteArray data);

private slots:
    // 串口槽函数
    void on_btnRefresh_Serial_clicked();
    void on_btnOpen_Serial_clicked();
    void on_btnClose_Serial_clicked();

    // 蓝牙槽函数
    void on_btnRefresh_Bt_clicked();
    void on_btnConnect_Bt_clicked();
    void on_btnDisconnect_Bt_clicked();

    // BLE专用槽函数
    void onBleDeviceFound(const QBluetoothDeviceInfo &deviceInfo);

    // 通用收发槽函数
    void on_btnClearRecv_clicked();
    void on_btnClearSend_clicked();
    void on_btnSend_clicked();

private:
    Ui::SerialTab *ui;
    SerialThread *m_serialThread;
    BluetoothThread *m_bluetoothThread;

    void initSerialPortList();
    void updateSerialUIState(bool isOpened);
    void updateBluetoothUIState(bool isConnected);
};

#endif // SERIALTAB_H