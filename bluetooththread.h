#ifndef BLUETOOTHTHREAD_H
#define BLUETOOTHTHREAD_H

#include <QObject>
#include <QBluetoothDeviceDiscoveryAgent>
#include <QBluetoothDeviceInfo>
#include <QLowEnergyController>
#include <QLowEnergyService>
#include <QLowEnergyCharacteristic>
#include <QLowEnergyDescriptor>
#include <QTime>

class BluetoothThread : public QObject
{
    Q_OBJECT
public:
    explicit BluetoothThread(QObject *parent = nullptr);

    void startBleDiscovery();
    void stopBleDiscovery();
    void connectToDevice(const QBluetoothDeviceInfo &info);
    void disconnectDevice();
    bool isConnected();
    void sendData(const QByteArray &data);

signals:
    void sig_bleDeviceFound(const QBluetoothDeviceInfo &info);
    void sig_bleDiscoveryFinished();
    void sig_connected();
    void sig_disconnected();
    void sig_bleError(const QString &msg);
    void sig_dataSort(QByteArray data);

private slots:
    // 搜索相关
    void onDeviceDiscovered(const QBluetoothDeviceInfo &info);
    void onDiscoveryFinished();
    void onDiscoveryError(QBluetoothDeviceDiscoveryAgent::Error error);

    // 连接相关
    void onConnected();
    void onDisconnected();
    void onControllerError(QLowEnergyController::Error error);

    // 服务发现相关
    void onServiceDiscovered(const QBluetoothUuid &uuid);
    void onServiceScanDone();
    void onServiceStateChanged(QLowEnergyService::ServiceState state);
    void onCharacteristicChanged(const QLowEnergyCharacteristic &c, const QByteArray &v);
    void onDescriptorWritten(const QLowEnergyDescriptor &desc, const QByteArray &value);

private:
    QBluetoothDeviceDiscoveryAgent *m_agent;
    QLowEnergyController *m_ctrl;
    QLowEnergyService *m_service;
    QLowEnergyCharacteristic m_tx;
    QLowEnergyCharacteristic m_rx;
    bool m_connected;
};

#endif // BLUETOOTHTHREAD_H