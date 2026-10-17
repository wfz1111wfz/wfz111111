#include "BluetoothThread.h"
#include <QDebug>
#include <QTimer>
#include <QIODevice>

BluetoothThread::BluetoothThread(QObject *parent) : QObject(parent), m_connected(false)
{
    m_agent = new QBluetoothDeviceDiscoveryAgent(this);//创建BLE设备搜索代理
    m_agent->setLowEnergyDiscoveryTimeout(45000);//搜索超时4秒

    // 连接搜索信号，搜到设备，搜索结束，搜索出错，
    connect(m_agent, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered,
            this, &BluetoothThread::onDeviceDiscovered, Qt::UniqueConnection);
    connect(m_agent, &QBluetoothDeviceDiscoveryAgent::finished,
            this, &BluetoothThread::onDiscoveryFinished, Qt::UniqueConnection);
    connect(m_agent, SIGNAL(error(QBluetoothDeviceDiscoveryAgent::Error)),
            this, SLOT(onDiscoveryError(QBluetoothDeviceDiscoveryAgent::Error)), Qt::UniqueConnection);
}

void BluetoothThread::startBleDiscovery() {
    if (m_agent->isActive())
        m_agent->stop();//如果在搜就停掉重新来
    qDebug() << "[BLE] 开始搜索设备...";
    m_agent->start(QBluetoothDeviceDiscoveryAgent::LowEnergyMethod);//QT底层函数搜索BLE设备
}

void BluetoothThread::stopBleDiscovery() {
    if (m_agent->isActive())
        m_agent->stop();//强制停止搜索
}

void BluetoothThread::connectToDevice(const QBluetoothDeviceInfo &info)
{
    disconnectDevice();
    qDebug() << "[BLE] 正在连接设备：" << info.name() << " MAC：" << info.address().toString();

    m_ctrl = QLowEnergyController::createCentral(info, this);//QT底层创建BLE中央控制器
    m_ctrl->setRemoteAddressType(QLowEnergyController::PublicAddress);//设置地址类型

    // 连接信号，将底层的状态跳转到实际函数执行
    connect(m_ctrl, &QLowEnergyController::connected,
            this, &BluetoothThread::onConnected, Qt::UniqueConnection);//底层连接成功跳到连接函数
    connect(m_ctrl, &QLowEnergyController::disconnected,
            this, &BluetoothThread::onDisconnected, Qt::UniqueConnection);//底层断开跳到断开函数
    connect(m_ctrl, SIGNAL(error(QLowEnergyController::Error)),
            this, SLOT(onControllerError(QLowEnergyController::Error)), Qt::UniqueConnection);//底层连接出错，跳到出错函数
    connect(m_ctrl, &QLowEnergyController::serviceDiscovered,
            this, &BluetoothThread::onServiceDiscovered, Qt::UniqueConnection);//底层发现服务跳到发现服务函数
    connect(m_ctrl, &QLowEnergyController::discoveryFinished,
            this, &BluetoothThread::onServiceScanDone, Qt::UniqueConnection);//底层扫描完成跳到扫描完成函数

    m_ctrl->connectToDevice();
}

void BluetoothThread::disconnectDevice()
{
    m_connected = false;//标记为未连接
    if (m_agent) m_agent->disconnect(this);//断开搜索器信号
    //清理服务对象
    if (m_service) {
        m_service->disconnect(this);
        m_service->deleteLater();//删除对象
        m_service = nullptr;//
    }
    //清理控制器对象
    if (m_ctrl) {
        m_ctrl->disconnect(this);
        m_ctrl->disconnectFromDevice();//底层断开连接
        m_ctrl->deleteLater();
        m_ctrl = nullptr;
    }
    //清理特征值
    m_rx = QLowEnergyCharacteristic();
    m_tx = QLowEnergyCharacteristic();
    qDebug() << "[BLE] 已断开连接，释放所有资源";
}
//判断是否连接
bool BluetoothThread::isConnected()
{
    return m_connected;
}
//通过蓝牙发数据给飞控
void BluetoothThread::sendData(const QByteArray &data)
{
    //检查没连接或者发送特征值无效就不发
    if (!m_connected || !m_tx.isValid()) {
        qDebug() << "[BLE] 发送失败：未连接或发送特征值无效";
        return;
    }
    // 真正发数据
    m_service->writeCharacteristic(m_tx, data, QLowEnergyService::WriteWithoutResponse);//底层调用无响应发送，不需要回复
    qDebug() << "[BLE] 已发送数据：" << data.toHex(' ').toUpper();
}

// ==================== 搜索相关 ====================
void BluetoothThread::onDeviceDiscovered(const QBluetoothDeviceInfo &info)
{
    qDebug() << "✅ 搜到BLE设备：" << info.name() << " | MAC：" << info.address().toString();
    emit sig_bleDeviceFound(info);//把搜到的设备发给界面
}
//搜索结束
void BluetoothThread::onDiscoveryFinished()
{
    qDebug() << "[BLE] 设备搜索结束";
    emit sig_bleDiscoveryFinished();
}
//搜索出错
void BluetoothThread::onDiscoveryError(QBluetoothDeviceDiscoveryAgent::Error error)
{
    Q_UNUSED(error);
    QString err = m_agent->errorString();//QT底层获取错误文字描述
    qDebug() << "[BLE] 搜索错误：" << err;
    emit sig_bleError(err);//发送错误给界面显示
}

// ==================== 连接相关 ====================
void BluetoothThread::onConnected()
{
    qDebug() << "✅ BLE底层连接成功，等待握手完成...";
    // Qt定时器延时2秒，等硬件握手稳定
    // 2 秒后，调用 Qt 底层的 discoverServices 开始找服务，串口服务（FFE0）
    QTimer::singleShot(2000, m_ctrl, &QLowEnergyController::discoverServices);
}
//断开
void BluetoothThread::onDisconnected()
{
    qDebug() << "[BLE] 连接已断开";
    disconnectDevice();//清理函数
    emit sig_disconnected();//发送到界面断开了
}
//连接出错
void BluetoothThread::onControllerError(QLowEnergyController::Error error)
{
    Q_UNUSED(error);
    QString err = m_ctrl->errorString();//底层获取错误文字
    qDebug() << "[BLE] 控制器错误：" << err;
    emit sig_bleError(err);
    disconnectDevice();
}

// ==================== 服务发现相关 ====================
void BluetoothThread::onServiceDiscovered(const QBluetoothUuid &uuid)
{
    qDebug() << "[BLE] 发现服务：" << uuid.toString();
}

void BluetoothThread::onServiceScanDone()
{
    qDebug() << "[BLE] 服务扫描完成，共发现服务：" << m_ctrl->services().size();

    // FFE0串口服务UUID
    QBluetoothUuid srvUuid(QString("0000FFE0-0000-1000-8000-00805F9B34FB"));
    if (!m_ctrl->services().contains(srvUuid)) {
        qDebug() << "❌ 未找到FFE0串口服务！";
        emit sig_bleError("未找到串口服务，设备不兼容");
        disconnectDevice();
        return;
    }
    qDebug() << "✅ 找到FFE0串口服务，加载详情...";

    m_service = m_ctrl->createServiceObject(srvUuid, this);
    if (!m_service) {
        qDebug() << "❌ 服务对象创建失败！";
        emit sig_bleError("服务创建失败");
        disconnectDevice();
        return;
    }

    // 连接服务信号
    connect(m_service, &QLowEnergyService::stateChanged,
            this, &BluetoothThread::onServiceStateChanged, Qt::UniqueConnection);//状态改变，调用特征值函数连接
    connect(m_service, &QLowEnergyService::characteristicChanged,
            this, &BluetoothThread::onCharacteristicChanged, Qt::UniqueConnection);//特征值改变，发给解析函数
    connect(m_service, &QLowEnergyService::descriptorWritten,
            this, &BluetoothThread::onDescriptorWritten, Qt::UniqueConnection);//Notify是否打开

    m_service->discoverDetails();
}

// ==================== 服务详情加载 ====================
void BluetoothThread::onServiceStateChanged(QLowEnergyService::ServiceState state)
{
    if (state == QLowEnergyService::ServiceDiscovered) {
        qDebug() << "✅ 服务详情加载完成，开始查找特征值...";

        // 发送和接收用分开的特征值
        // 发送特征值：FFE1
        QBluetoothUuid txUuid(QString("0000FFE1-0000-1000-8000-00805F9B34FB"));
        m_tx = m_service->characteristic(txUuid);
        // 接收特征值：FFE2
        QBluetoothUuid rxUuid(QString("0000FFE2-0000-1000-8000-00805F9B34FB"));
        m_rx = m_service->characteristic(rxUuid);

        // 校验特征值是否有效
        if (!m_rx.isValid() || !m_tx.isValid()) {
            qDebug() << "❌ 未找到有效特征值！当前服务下所有特征值：";
            QList<QLowEnergyCharacteristic> chars = m_service->characteristics();
            for(int i=0; i<chars.size(); i++) {
                const QLowEnergyCharacteristic &c = chars.at(i);
                qDebug() << "  特征值" << i << "：" << c.uuid().toString() << " | 属性：" << c.properties();
            }
            emit sig_bleError("未找到串口特征值");
            disconnectDevice();
            return;
        }
        qDebug() << "✅ 找到发送特征值FFE1，属性：" << m_tx.properties();
        qDebug() << "✅ 找到接收特征值FFE2，属性：" << m_rx.properties();

        // 校验接收特征值是否支持Notify
        if (!(m_rx.properties() & QLowEnergyCharacteristic::Notify)) {
            qDebug() << "❌ 接收特征值不支持Notify，无法接收数据！";
            emit sig_bleError("设备不支持数据通知");
            disconnectDevice();
            return;
        }

        qDebug() << "[BLE] 正在开启Notify接收数据...";
        //QT获取 CCCD 描述符（开启 Notify 的开关
        QLowEnergyDescriptor cccd = m_rx.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration);
        if (!cccd.isValid()) {
            qDebug() << "❌ 找不到CCCD描述符！";
            emit sig_bleError("无法开启数据通知");
            disconnectDevice();
            return;
        }

        // 写入0100开启Notify，飞控发数据，电脑自动收到
        m_service->writeDescriptor(cccd, QByteArray::fromHex("0100"));
    }
}

// ==================== Notify开启结果回调 ====================
void BluetoothThread::onDescriptorWritten(const QLowEnergyDescriptor &desc, const QByteArray &value)
{   // 判断：是不是 CCCD 描述符
    if (desc.uuid() == QBluetoothUuid::ClientCharacteristicConfiguration) {
         // 判断：写入的值是不是 0100
        if (value == QByteArray::fromHex("0100")) {
            qDebug() << "✅ Notify开启成功！BLE连接完全建立，可以收发数据了";
            m_connected = true;//标记为连接
            emit sig_connected();
        } else {
            qDebug() << "❌ Notify开启失败！";
            emit sig_bleError("蓝牙连接失败");
            disconnectDevice();
        }
    }
}

// ==================== 接收飞控数据 ====================
void BluetoothThread::onCharacteristicChanged(const QLowEnergyCharacteristic &c, const QByteArray &v)
{
    if (c.uuid() == m_rx.uuid()) {
        qDebug() << "[BLE] 收到原始数据：" << v.toHex(' ').toUpper() << " | 长度：" << v.size();
        emit sig_dataSort(v); // 直接对接现有的解析逻辑
    }
}