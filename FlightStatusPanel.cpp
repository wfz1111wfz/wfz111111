#include "FlightStatusPanel.h"

FlightStatusPanel::FlightStatusPanel(QWidget *parent)
    : QWidget(parent)
{
    QGroupBox *groupBox = new QGroupBox("飞行状态", this);//飞行模式布局
    QVBoxLayout *layout = new QVBoxLayout(groupBox);

    lblFlyMode = new QLabel("飞行模式：--");//初始化状态数据
    lblArmed = new QLabel("解锁状态：已锁定");
    lblAltitude = new QLabel("高度：0 cm");
    lblVSpeed = new QLabel("垂直速度：0.00 m/s");
    lblASpeed = new QLabel("水平速度：0.00 m/s");
    lblRoll = new QLabel("Roll：0.00 °");
    lblPitch = new QLabel("Pitch：0.00 °");
    lblYaw = new QLabel("Yaw：0.00 °");

    layout->addWidget(lblFlyMode);//布局器
    layout->addWidget(lblArmed);
    layout->addWidget(lblAltitude);
    layout->addWidget(lblVSpeed);
    layout->addWidget(lblASpeed);
    layout->addWidget(lblRoll);
    layout->addWidget(lblPitch);
    layout->addWidget(lblYaw);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(groupBox);
    this->setLayout(mainLayout);
}

void FlightStatusPanel::updateStatus(uint8_t flyModel, uint8_t armed, float alt,
                                     float vsp, float asp, float roll, float pitch, float yaw)
{
    // ====================== 直接显示数字模式 ======================
    lblFlyMode->setText(QString("飞行模式：%1").arg(flyModel));

    // ============================================
    /*
    QString modeStr;
    switch(flyModel) {
    case 1: modeStr = "手动模式"; break;
    case 2: modeStr = "自稳模式"; break;
    case 3: modeStr = "定高模式"; break;
    case 4: modeStr = "定点模式"; break;
    case 5: modeStr = "返航模式"; break;
    case 6: modeStr = "特技模式"; break;
    default: modeStr = QString("未知模式(%1)").arg(flyModel); break;
    }
    lblFlyMode->setText("飞行模式：" + modeStr);
    */
    // ==================================================================================

    // 解锁状态
    if(armed == 1)
        lblArmed->setText("解锁状态：已解锁");
    else
        lblArmed->setText("解锁状态：已锁定");

    lblAltitude->setText(QString("高度：%1 cm").arg(alt, 0, 'f', 0));//保留小数位
    lblVSpeed->setText(QString("垂直速度：%1 m/s").arg(vsp, 0, 'f', 2));
    lblASpeed->setText(QString("水平速度：%1 m/s").arg(asp, 0, 'f', 2));
    lblRoll->setText(QString("Roll：%1 °").arg(roll, 0, 'f', 1));
    lblPitch->setText(QString("Pitch：%1 °").arg(pitch, 0, 'f', 1));
    lblYaw->setText(QString("Yaw：%1 °").arg(yaw, 0, 'f', 1));
}

// 读取飞控返回后，直接显示模式文字
void FlightStatusPanel::setFlyModeText(const QString &text)
{
    lblFlyMode->setText("飞行模式：" + text);
}