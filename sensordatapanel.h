#ifndef SENSORDATAPANEL_H
#define SENSORDATAPANEL_H

#include <QWidget>
#include <QGroupBox>
#include <QFormLayout>
#include <QLabel>
#include <QVBoxLayout>

class SensorDataPanel : public QWidget
{
    Q_OBJECT
public:
    explicit SensorDataPanel(QWidget *parent = nullptr);

public slots:
    // 原有传感器数据更新函数（保留不变，兼容原有逻辑）
    void updateSensorData(float accX, float accY, float accZ, float gyrX, float gyrY, float gyrZ,
                           int magX, int magY, int magZ, float baroAlt, float sonarAlt,
                           float speedX, float speedY, float speedZ);
    // 【新增】电压电流更新函数（单独更新，适配独立的0x05功能字帧）
    void updatePowerData(float voltage, float current);

private:
    void initUI();

    // 原有传感器标签
    QLabel *m_lbAccX;
    QLabel *m_lbAccY;
    QLabel *m_lbAccZ;
    QLabel *m_lbGyrX;
    QLabel *m_lbGyrY;
    QLabel *m_lbGyrZ;
    QLabel *m_lbMagX;
    QLabel *m_lbMagY;
    QLabel *m_lbMagZ;
    QLabel *m_lbBaroAlt;
    QLabel *m_lbSonarAlt;
    QLabel *m_lbSpeedX;
    QLabel *m_lbSpeedY;
    QLabel *m_lbSpeedZ;
    QLabel *m_lbVoltage;
    QLabel *m_lbCurrent;
};

#endif // SENSORDATAPANEL_H
