#include "ReceiverPanel.h"

// ====================== 可配置参数，根据你的摇杆实际量程调整 ======================
const int PWM_MIN = 1000;    // 摇杆最小PWM值（摇到最底/最左的数值）
const int PWM_MID = 1500;    // 摇杆中位PWM值
const int PWM_MAX = 2000;    // 摇杆最大PWM值（摇到最顶/最右的数值）
const float ANGLE_MAX = 45.0f; // 摇杆满行程对应的最大角度
// ==================================================================================

ReceiverPanel::ReceiverPanel(QWidget *parent)
    : QWidget(parent)
{
    initUI();
}

void ReceiverPanel::initUI()
{
    QGroupBox *groupBox = new QGroupBox("接收机", this);
    QFormLayout *formLayout = new QFormLayout(groupBox);
    // 统一布局间距，和传感器面板对齐
    formLayout->setSpacing(8);
    formLayout->setContentsMargins(10,15,10,15);

    // 初始化标签，默认值0
    m_lbThr = new QLabel("0");
    m_lbYaw = new QLabel("0");
    m_lbRol = new QLabel("0");
    m_lbPit = new QLabel("0");
    m_lbAux1 = new QLabel("0");
    m_lbAux2 = new QLabel("0");
    m_lbAux3 = new QLabel("0");
    m_lbAux4 = new QLabel("0");
    m_lbAux5 = new QLabel("0");
    m_lbAux6 = new QLabel("0");

    // 添加到布局
    formLayout->addRow("THR", m_lbThr);
    formLayout->addRow("YAW", m_lbYaw);
    formLayout->addRow("ROL", m_lbRol);
    formLayout->addRow("PIT", m_lbPit);
    formLayout->addRow("AUX1", m_lbAux1);
    formLayout->addRow("AUX2", m_lbAux2);
    formLayout->addRow("AUX3", m_lbAux3);
    formLayout->addRow("AUX4", m_lbAux4);
    formLayout->addRow("AUX5", m_lbAux5);
    formLayout->addRow("AUX6", m_lbAux6);

    // 主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(groupBox);
    mainLayout->setContentsMargins(0,0,0,0);
    mainLayout->setSpacing(0);
    this->setLayout(mainLayout);
}

// PWM值转角度的通用函数
static float pwmToAngle(int rawPwm)
{
    // 先限幅，避免超量程导致角度异常
    int clampedPwm = qBound(PWM_MIN, rawPwm, PWM_MAX);
    float angle;

    // 区分正负行程，适配摇杆左右/上下不对称的情况
    if (clampedPwm >= PWM_MID) {
        // 正行程：中位→最大值，映射成 0→+ANGLE_MAX
        angle = (clampedPwm - PWM_MID) * 1.0f / (PWM_MAX - PWM_MID) * ANGLE_MAX;
    } else {
        // 负行程：中位→最小值，映射成 0→-ANGLE_MAX
        angle = (clampedPwm - PWM_MID) * 1.0f / (PWM_MID - PWM_MIN) * ANGLE_MAX;
    }

    return angle;
}

void ReceiverPanel::updateChannelData(int thr, int yaw, int rol, int pit, int aux1, int aux2, int aux3, int aux4, int aux5, int aux6)
{
    // 所有摇杆通道统一转角度
    float thr_angle = pwmToAngle(thr);
    float yaw_angle = pwmToAngle(yaw);
    float rol_angle = pwmToAngle(rol);
    float pit_angle = pwmToAngle(pit);

    // 所有通道统一显示角度，保留1位小数
    m_lbThr->setText(QString::number(thr_angle, 'f', 1) + "°");
    m_lbYaw->setText(QString::number(yaw_angle, 'f', 1) + "°");
    m_lbRol->setText(QString::number(rol_angle, 'f', 1) + "°");
    m_lbPit->setText(QString::number(pit_angle, 'f', 1) + "°");

    // 辅助通道：保持显示原始PWM值（开关不需要角度）
    m_lbAux1->setText(QString::number(aux1));
    m_lbAux2->setText(QString::number(aux2));
    m_lbAux3->setText(QString::number(aux3));
    m_lbAux4->setText(QString::number(aux4));
    m_lbAux5->setText(QString::number(aux5));
    m_lbAux6->setText(QString::number(aux6));
}