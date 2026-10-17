#ifndef FLIGHTSTATUSPANEL_H
#define FLIGHTSTATUSPANEL_H

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QGroupBox>

class FlightStatusPanel : public QWidget
{
    Q_OBJECT
public:
    explicit FlightStatusPanel(QWidget *parent = nullptr);

    // 常规状态更新
    void updateStatus(uint8_t flyModel, uint8_t armed, float alt,
                      float vsp, float asp, float roll, float pitch, float yaw);

    // 专门用于显示读取到的飞行模式文字
    void setFlyModeText(const QString &text);

private:
    QLabel *lblFlyMode;
    QLabel *lblArmed;
    QLabel *lblAltitude;
    QLabel *lblVSpeed;
    QLabel *lblASpeed;
    QLabel *lblRoll;
    QLabel *lblPitch;
    QLabel *lblYaw;
};

#endif // FLIGHTSTATUSPANEL_H
