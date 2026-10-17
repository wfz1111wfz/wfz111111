#include "GpsInfoPanel.h"

GpsInfoPanel::GpsInfoPanel(QWidget *parent)
    : QWidget(parent)
{
    initUI();
}

void GpsInfoPanel::initUI()
{
    QGroupBox *groupBox = new QGroupBox("GPS信息", this);
    QFormLayout *formLayout = new QFormLayout(groupBox);

    // 初始化标签
    m_lbLon = new QLabel("0.00000000");
    m_lbLat = new QLabel("0.00000000");
    m_lbFix = new QLabel("3D-FIX");
    m_lbSats = new QLabel("0");
    m_lbAlt = new QLabel("0.0 m");
    m_lbSpeedN = new QLabel("0.0 cm/s");
    m_lbSpeedE = new QLabel("0.0 cm/s");
    m_lbYaw = new QLabel("0.00 °");

    // 添加到布局
    formLayout->addRow("经度", m_lbLon);
    formLayout->addRow("纬度", m_lbLat);
    formLayout->addRow("定位状态", m_lbFix);
    formLayout->addRow("卫星数量", m_lbSats);
    formLayout->addRow("海拔高度", m_lbAlt);
    formLayout->addRow("速度-N", m_lbSpeedN);
    formLayout->addRow("速度-E", m_lbSpeedE);
    formLayout->addRow("回航角", m_lbYaw);

    // 主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(groupBox);
    mainLayout->setContentsMargins(0,0,0,0);
}

void GpsInfoPanel::updateGpsData(double lon, double lat, int fixType, int satsNum, float alt, float speedN, float speedE, float yaw)
{
    m_lbLon->setText(QString::number(lon, 'f', 8));
    m_lbLat->setText(QString::number(lat, 'f', 8));
    m_lbSats->setText(QString::number(satsNum));
    m_lbAlt->setText(QString::number(alt, 'f', 1) + " m");
    m_lbSpeedN->setText(QString::number(speedN, 'f', 1) + " cm/s");
    m_lbSpeedE->setText(QString::number(speedE, 'f', 1) + " cm/s");
    m_lbYaw->setText(QString::number(yaw, 'f', 2) + " °");
}
