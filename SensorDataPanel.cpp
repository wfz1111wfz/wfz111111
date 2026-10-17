#include "SensorDataPanel.h"

SensorDataPanel::SensorDataPanel(QWidget *parent)
    : QWidget(parent)
{
    initUI();
}

void SensorDataPanel::initUI()
{
    QGroupBox *groupBox = new QGroupBox("传感器数据", this);
    QFormLayout *formLayout = new QFormLayout(groupBox);
    formLayout->setSpacing(8);
    formLayout->setContentsMargins(10,15,10,15);

    // 原有传感器标签初始化
    m_lbAccX = new QLabel("0.00 g");
    m_lbAccY = new QLabel("0.00 g");
    m_lbAccZ = new QLabel("0.00 g");
    m_lbGyrX = new QLabel("0.00 °/s");
    m_lbGyrY = new QLabel("0.00 °/s");
    m_lbGyrZ = new QLabel("0.00 °/s");
    m_lbMagX = new QLabel("0 mGauss");
    m_lbMagY = new QLabel("0 mGauss");
    m_lbMagZ = new QLabel("0 mGauss");
    m_lbBaroAlt = new QLabel("0.00 m");
    m_lbSonarAlt = new QLabel("0.00 m");
    m_lbSpeedX = new QLabel("0.00 m/s");
    m_lbSpeedY = new QLabel("0.00 m/s");
    m_lbSpeedZ = new QLabel("0.00 m/s");

    // 【新增】电压电流标签初始化
    m_lbVoltage = new QLabel("0.00 V");
    m_lbCurrent = new QLabel("0.00 A");

    // 添加到布局（和你截图的顺序完全一致，电压电流放在最后）
    formLayout->addRow("ACC-X", m_lbAccX);
    formLayout->addRow("ACC-Y", m_lbAccY);
    formLayout->addRow("ACC-Z", m_lbAccZ);
    formLayout->addRow("GYR-X", m_lbGyrX);
    formLayout->addRow("GYR-Y", m_lbGyrY);
    formLayout->addRow("GYR-Z", m_lbGyrZ);
    formLayout->addRow("MAG-X", m_lbMagX);
    formLayout->addRow("MAG-Y", m_lbMagY);
    formLayout->addRow("MAG-Z", m_lbMagZ);
    formLayout->addRow("气压高度", m_lbBaroAlt);
    formLayout->addRow("超声高度", m_lbSonarAlt);
    formLayout->addRow("速度-X", m_lbSpeedX);
    formLayout->addRow("速度-Y", m_lbSpeedY);
    formLayout->addRow("速度-Z", m_lbSpeedZ);
    // 【新增】电压电流行
    formLayout->addRow("电池电压", m_lbVoltage);
    formLayout->addRow("工作电流", m_lbCurrent);

    // 主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(groupBox);
    mainLayout->setContentsMargins(0,0,0,0);
    mainLayout->setSpacing(0);
    this->setLayout(mainLayout);
}

// 原有传感器数据更新函数（完全保留，不影响原有逻辑）
void SensorDataPanel::updateSensorData(float accX, float accY, float accZ, float gyrX, float gyrY, float gyrZ, int magX, int magY, int magZ, float baroAlt, float sonarAlt, float speedX, float speedY, float speedZ)
{
    m_lbAccX->setText(QString::number(accX, 'f', 2) + " g");
    m_lbAccY->setText(QString::number(accY, 'f', 2) + " g");
    m_lbAccZ->setText(QString::number(accZ, 'f', 2) + " g");

    m_lbGyrX->setText(QString::number(gyrX, 'f', 2) + " °/s");
    m_lbGyrY->setText(QString::number(gyrY, 'f', 2) + " °/s");
    m_lbGyrZ->setText(QString::number(gyrZ, 'f', 2) + " °/s");

    m_lbMagX->setText(QString::number(magX) + " mGauss");
    m_lbMagY->setText(QString::number(magY) + " mGauss");
    m_lbMagZ->setText(QString::number(magZ) + " mGauss");

    m_lbBaroAlt->setText(QString::number(baroAlt, 'f', 2) + " m");
    m_lbSonarAlt->setText(QString::number(sonarAlt, 'f', 2) + " m");
    m_lbSpeedX->setText(QString::number(speedX, 'f', 2) + " m/s");
    m_lbSpeedY->setText(QString::number(speedY, 'f', 2) + " m/s");
    m_lbSpeedZ->setText(QString::number(speedZ, 'f', 2) + " m/s");
}

// 电压电流更新函数
void SensorDataPanel::updatePowerData(float voltage, float current)
{
    // 电压：单位V，保留2位小数（原始数据*100，这里除以100还原）
    m_lbVoltage->setText(QString::number(voltage, 'f', 2) + " V");
    // 电流：单位A，保留2位小数（原始数据*100，这里除以100还原）
    m_lbCurrent->setText(QString::number(current, 'f', 2) + " A");
}
