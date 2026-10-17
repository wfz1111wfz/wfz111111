#ifndef FLIGHTSTATUSTAB_H
#define FLIGHTSTATUSTAB_H

#include <QWidget>
#include "SerialThread.h"
#include "AttitudeDialWidget.h"
#include "Drone3DView.h"
#include "AttitudeWaveChart.h"
#include "FlightStatusPanel.h"
#include "ReceiverPanel.h"
#include "SensorDataPanel.h"
#include "GpsInfoPanel.h"

class SerialTab;

class FlightStatusTab : public QWidget
{
    Q_OBJECT

public:
    explicit FlightStatusTab(QWidget *parent = nullptr, SerialTab *serialTab = nullptr);
    ~FlightStatusTab();
    void setSerialThread(SerialThread* thread);

private:
    SerialTab* m_serialTab;
    SerialThread* m_serialThread;

    Drone3DView *m_drone3DView;
    AttitudeDialWidget *m_attitudeDial;
    AttitudeWaveChart *m_attitudeWave;
    FlightStatusPanel *m_statusPanel;
    ReceiverPanel *m_receiverPanel;
    SensorDataPanel *m_sensorPanel;
    GpsInfoPanel *m_gpsPanel;

    void initLayout();
    bool verifyChecksum(const QByteArray &data, int checksumPos);
    void parseStatusFrame(const QByteArray &payload);
    void parseSensorFrame(const QByteArray &payload);
    void parseRcDataFrame(const QByteArray &payload);
    void parsePowerFrame(const QByteArray &payload);
    void parseSensor2Frame(const QByteArray &payload);
    void parseFlyModeAckFrame(const QByteArray &payload);

    static const uint8_t FRAME_HEADER_AA = 0xAA;
    static const uint8_t FUNC_ID_ATTITUDE = 0x01;
    static const uint8_t FUNC_ID_SENSOR = 0x02;
    static const uint8_t FUNC_ID_RCDATA = 0x03;
    static const uint8_t FUNC_ID_SENSOR2 = 0x07;
    static const uint8_t FUNC_ID_POWER = 0x05;
    static const uint8_t FUNC_ID_FLY_MODE_ACK = 0x0A;

    static const uint8_t PAYLOAD_LEN_ATTITUDE = 12;
    static const uint8_t PAYLOAD_LEN_SENSOR = 18;
    static const uint8_t PAYLOAD_LEN_RCDATA = 20;
    static const uint8_t PAYLOAD_LEN_SENSOR2 = 6;
    static const uint8_t PAYLOAD_LEN_POWER = 4;
    static const uint8_t PAYLOAD_LEN_FLY_MODE_ACK = 1;


public slots:
    void parseDataFrame(QByteArray rxFrame); // 👈 现在是公有槽函数，能连接了

};

#endif // FLIGHTSTATUSTAB_H