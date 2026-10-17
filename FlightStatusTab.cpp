#include "FlightStatusTab.h"
#include <QDataStream>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDebug>

extern SerialConfig serialConfig;

FlightStatusTab::FlightStatusTab(QWidget *parent, SerialTab *serialTab)
    : QWidget(parent)
    , m_serialTab(serialTab)
    , m_serialThread(nullptr)
{
    initLayout();//ui界面创建
}

FlightStatusTab::~FlightStatusTab()
{
    if(m_serialThread) {
        disconnect(m_serialThread, nullptr, this, nullptr);// 如果串口线程还存在，先断开所有信号槽连接，防止野指针
    }
}
// ====================== 绑定串口线程 ======================
void FlightStatusTab::setSerialThread(SerialThread* thread)
{
    if(!thread) return; // 安全检查：如果传进来是空指针，直接返回

// 如果之前已经绑定过线程，先断开旧的连接
    if(m_serialThread) {
        disconnect(m_serialThread, nullptr, this, nullptr);
    }

    m_serialThread = thread;// 保存新的线程指针

    connect(m_serialThread, &SerialThread::sig_dataSort,
            this, &FlightStatusTab::parseDataFrame,
            Qt::QueuedConnection);// 当串口线程解析好一帧数据（sig_dataSort），就发给 parseDataFrame 处理

    qDebug() << "[FlightStatus] 串口线程已绑定";
}

void FlightStatusTab::initLayout()
{
    // ---顶部布局（上半部分） ---
    QHBoxLayout *topLayout = new QHBoxLayout();

    m_drone3DView = new Drone3DView(this);
    m_attitudeDial = new AttitudeDialWidget(this); // 创建两个核心显示控件：3D飞机视图 和 姿态仪表盘
    m_drone3DView->setMinimumSize(450, 350);
    m_attitudeDial->setMinimumSize(450, 350);  // 设置最小尺寸，防止它们被挤得太小
    topLayout->addWidget(m_drone3DView);
    topLayout->addWidget(m_attitudeDial);// 把它们加到顶部布局里（左右排列）
    // --- 底部布局（下半部分） ---
    QHBoxLayout *bottomLayout = new QHBoxLayout();
    // 创建所有的数据面板
    m_statusPanel = new FlightStatusPanel(this);// 飞行状态面板
    m_attitudeWave = new AttitudeWaveChart(this); // 姿态波形图
    m_receiverPanel = new ReceiverPanel(this);// 接收机数据面板
    m_sensorPanel = new SensorDataPanel(this);// 传感器数据面板
    m_gpsPanel = new GpsInfoPanel(this);// GPS信息面板

    m_attitudeWave->setMinimumSize(400, 360);// 把这些面板从左到右依次排开
    bottomLayout->addWidget(m_statusPanel);
    bottomLayout->addWidget(m_attitudeWave);
    bottomLayout->addWidget(m_receiverPanel);
    bottomLayout->addWidget(m_sensorPanel);
    bottomLayout->addWidget(m_gpsPanel);
// --- 3. 主布局---
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(topLayout);
    mainLayout->addLayout(bottomLayout);
    // 设置间距和边距
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(10,10,10,10);
    this->setLayout(mainLayout); // 应用主布局
}

void FlightStatusTab::parseDataFrame(QByteArray rxFrame)
{
    qDebug() << "[FlightStatus] 收到完整帧:" << rxFrame.toHex(' ').toUpper();

    if(rxFrame.size() < 5) {
        qDebug() << "[FlightStatus] 帧长度过短，丢弃";
        return;
    }

    if((uint8_t)rxFrame[0] != FRAME_HEADER_AA || (uint8_t)rxFrame[1] != FRAME_HEADER_AA) {
        qDebug() << "[FlightStatus] 帧头不匹配，丢弃";//帧头AA AA判断
        return;
    }

    uint8_t funcId = (uint8_t)rxFrame[2];//功能字
    uint8_t payloadLen = (uint8_t)rxFrame[3];//数据长度
    int expectedTotalLen = 4 + payloadLen + 1;//总长

    if(rxFrame.size() != expectedTotalLen) {
        qDebug() << "[FlightStatus] 总长度不匹配! 期望:" << expectedTotalLen << "实际:" << rxFrame.size();
        return;//判断接收到的帧和实际的长度是否一致
    }

    if(!verifyChecksum(rxFrame, expectedTotalLen - 1)) {
        qDebug() << "[FlightStatus] 校验和错误，丢弃";
        return;//判断校验和
    }

    QByteArray payload = rxFrame.mid(4, payloadLen);
    qDebug() << "[FlightStatus] 功能码:0x" << QString::number(funcId, 16).toUpper() << "Payload长度:" << payloadLen;
//判断对应的功能字跳转到对应的解析函数
    if(funcId == FUNC_ID_ATTITUDE && payloadLen == PAYLOAD_LEN_ATTITUDE) {
        parseStatusFrame(payload);//姿态角
    }
    else if(funcId == FUNC_ID_SENSOR && payloadLen == PAYLOAD_LEN_SENSOR) {
        parseSensorFrame(payload);//传感器
    }
    else if(funcId == FUNC_ID_RCDATA && payloadLen == PAYLOAD_LEN_RCDATA) {
        parseRcDataFrame(payload);//遥控器
    }
    else if(funcId == FUNC_ID_SENSOR2 && payloadLen == PAYLOAD_LEN_SENSOR2) {
        parseSensor2Frame(payload);//高度传感器
    }
    else if(funcId == FUNC_ID_POWER && payloadLen == PAYLOAD_LEN_POWER) {
        parsePowerFrame(payload);//电压电流
    }
    else if(funcId == FUNC_ID_FLY_MODE_ACK && payloadLen == PAYLOAD_LEN_FLY_MODE_ACK) {
        parseFlyModeAckFrame(payload);
    }
    else {
        qDebug() << "[FlightStatus] 未知功能码或Payload长度不匹配，丢弃";
    }
}

bool FlightStatusTab::verifyChecksum(const QByteArray &data, int checksumPos)//校验和函数
{
    uint8_t sum = 0;
    for(int i = 0; i < checksumPos; i++) {
        sum += (uint8_t)data[i];
    }
    return (sum == (uint8_t)data[checksumPos]);//累加取低八位比较校验和
}

void FlightStatusTab::parseStatusFrame(const QByteArray &payload)
{
    QDataStream stream(payload);
    stream.setByteOrder(QDataStream::BigEndian);

    int16_t rol_raw, pit_raw, yaw_raw;//定义变量接收原始数据（int16_t, int32_t）
    int32_t alt_raw;
    uint8_t fly_model, armed;

    stream >> rol_raw >> pit_raw >> yaw_raw >> alt_raw >> fly_model >> armed;

    float rol = rol_raw / 100.0f;//角度原始值 / 100 = 实际角度（度）
    float pit = pit_raw / 100.0f;
    float yaw = yaw_raw / 100.0f;
    float alt = alt_raw;

    qDebug() << "[FlightStatus] 解析姿态数据: Roll=" << rol << " Pitch=" << pit << " Yaw=" << yaw << " Alt=" << alt;

    if(m_drone3DView) m_drone3DView->setAttitude(rol, pit, yaw);//更新3d
    if(m_attitudeDial) m_attitudeDial->setAttitude(rol, pit, yaw);//更新仪表盘
    if(m_attitudeWave) m_attitudeWave->updateData(rol, pit, yaw);//更新波形图
    if(m_statusPanel) m_statusPanel->updateStatus(fly_model, armed, alt, 0, 0, rol, pit, yaw);//更新文字面板
}

void FlightStatusTab::parseSensorFrame(const QByteArray &payload)
{
    QDataStream stream(payload);
    stream.setByteOrder(QDataStream::BigEndian);

    int16_t accX, accY, accZ;
    int16_t gyrX, gyrY, gyrZ;
    int16_t magX, magY, magZ;

    stream >> accX >> accY >> accZ;
    stream >> gyrX >> gyrY >> gyrZ;
    stream >> magX >> magY >> magZ;

    float ax = accX / 1000.0f;
    float ay = accY / 1000.0f;
    float az = accZ / 1000.0f;
    float gx = gyrX / 100.0f;
    float gy = gyrY / 100.0f;
    float gz = gyrZ / 100.0f;

    qDebug() << "[FlightStatus] 解析传感器数据: Acc=" << ax << ay << az << " Gyr=" << gx << gy << gz;

    if(m_sensorPanel) {
        m_sensorPanel->updateSensorData(ax, ay, az, gx, gy, gz,
                                        magX, magY, magZ,
                                        0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
    }
}

void FlightStatusTab::parseRcDataFrame(const QByteArray &payload)
{
    QDataStream stream(payload);
    stream.setByteOrder(QDataStream::BigEndian);

    int16_t thr, yaw, rol, pit;
    int16_t aux1, aux2, aux3, aux4, aux5, aux6;

    stream >> thr >> yaw >> rol >> pit;
    stream >> aux1 >> aux2 >> aux3 >> aux4 >> aux5 >> aux6;

    qDebug() << "[FlightStatus] 解析接收机数据: THR=" << thr << " YAW=" << yaw << " ROL=" << rol << " PIT=" << pit;

    if(m_receiverPanel) {
        m_receiverPanel->updateChannelData(thr, yaw, rol, pit, aux1, aux2, aux3, aux4, aux5, aux6);
    }
}

// ====================== 电压电流解析======================
void FlightStatusTab::parsePowerFrame(const QByteArray &payload)
{
    if(payload.size() < 4) return;

    QDataStream stream(payload);
    stream.setByteOrder(QDataStream::BigEndian);

    uint16_t voltageRaw, currentRaw;
    stream >> voltageRaw >> currentRaw;

    float voltage = voltageRaw / 100.0f;
    float current = currentRaw / 100.0f;

    qDebug() << "[FlightStatus] 解析电压电流: Voltage=" << voltage << "V, Current=" << current << "A";

    if(m_sensorPanel) {
        m_sensorPanel->updatePowerData(voltage, current);
    }
}

// ====================== 高度传感器解析（======================
void FlightStatusTab::parseSensor2Frame(const QByteArray &payload)
{
    if (payload.size() < 6) return;

    QDataStream stream(payload);
    stream.setByteOrder(QDataStream::BigEndian);

    int32_t altBarRaw;
    uint16_t altCsbRaw;
    stream >> altBarRaw >> altCsbRaw;

    float baroAlt = altBarRaw / 100.0f;
    float sonarAlt = altCsbRaw / 100.0f;

    qDebug() << "[FlightStatus] 解析高度数据: 气压=" << baroAlt << "m 超声=" << sonarAlt << "m";

    if (m_sensorPanel) {
        m_sensorPanel->updateSensorData(
            0, 0, 0, 0, 0, 0, 0, 0, 0,
            baroAlt, sonarAlt,
            0, 0, 0
            );
    }
}

// ====================== 飞行模式应答解析======================
void FlightStatusTab::parseFlyModeAckFrame(const QByteArray &payload)
{
    if (payload.size() < 1) return;
    uint8_t flyMode = (uint8_t)payload[0];

    QString modeStr;
    switch(flyMode) {
    case 1: modeStr = "手动模式"; break;
    case 2: modeStr = "自稳模式"; break;
    case 3: modeStr = "定高模式"; break;
    case 4: modeStr = "定点模式"; break;
    case 5: modeStr = "返航模式"; break;
    case 6: modeStr = "特技模式"; break;
    default: modeStr = QString("未知模式(%1)").arg(flyMode); break;
    }

    qDebug() << "[FlightStatus] 当前飞行模式：" << modeStr;

    // 直接更新到状态面板显示
    if (m_statusPanel) {
        m_statusPanel->setFlyModeText(modeStr);
    }
}