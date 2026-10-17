#include "CalibrateTab.h"
#include <QTime>
#include <QMessageBox>
#include <QDebug>

// 参数配置表（完全保留你的原内容）
struct ParamConfig {
    QString name;
    uint16_t id;
    QString unit;
    float scale;
    QString desc;
};
const QVector<ParamConfig> PARAM_LIST = {
    {"起飞高度", 0x30, "厘米", 100.0f, "单位：厘米。若飞控起飞时使用定高模式且该参数不为0，则飞行器自动升高至该高度值，并定高飞行。稳定到该高度后，可以通过遥控控制飞机的上升下降。"},
    };

CalibrateTab::CalibrateTab(SerialThread *serialThread, QWidget *parent)
    : QWidget(parent)
    , m_serialThread(serialThread)

{
    initUI();

    // 连接串口数据解析信号
    if (m_serialThread) {
        connect(m_serialThread, &SerialThread::sig_dataSort,
                this, &CalibrateTab::parseFlyModeData, Qt::QueuedConnection);
    }
}

CalibrateTab::~CalibrateTab()
{
}

void CalibrateTab::setSerialThread(SerialThread* thread)
{
    if(!thread) return;
    if(m_serialThread) disconnect(m_serialThread, nullptr, this, nullptr);

    m_serialThread = thread;
    connect(m_serialThread, &SerialThread::sig_dataSort,
            this, &CalibrateTab::parseFlyModeData, Qt::QueuedConnection);
}

void CalibrateTab::initUI()
{
    // ====================== 主布局 ======================
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    QHBoxLayout *topLayout = new QHBoxLayout();

    // ============== 左列：飞行模式区 ==============
    QGroupBox *groupMode = new QGroupBox("飞行模式");
    QVBoxLayout *modeLayout = new QVBoxLayout(groupMode);

    // AUX1
    QHBoxLayout *aux1Layout = new QHBoxLayout();
    aux1Layout->addWidget(new QLabel("AUX1"));
    QSlider *aux1Slider = new QSlider(Qt::Horizontal);
    aux1Slider->setRange(1000, 2000);
    aux1Slider->setValue(1500);
    QSpinBox *aux1Spin = new QSpinBox();
    aux1Spin->setRange(1000, 2000);
    aux1Spin->setValue(1500);
    connect(aux1Slider, &QSlider::valueChanged, aux1Spin, &QSpinBox::setValue);
    connect(aux1Spin, QOverload<int>::of(&QSpinBox::valueChanged), aux1Slider, &QSlider::setValue);
    aux1Layout->addWidget(aux1Slider);
    aux1Layout->addWidget(aux1Spin);
    modeLayout->addLayout(aux1Layout);

    QHBoxLayout *aux1ModeLayout = new QHBoxLayout();
    aux1ModeLayout->addWidget(new QLabel("模式"));
    m_cmbAux1Low = new QComboBox();
    m_cmbAux1Mid = new QComboBox();
    m_cmbAux1High = new QComboBox();
    initFlyModeComboBox(m_cmbAux1Low);
    initFlyModeComboBox(m_cmbAux1Mid);
    initFlyModeComboBox(m_cmbAux1High);
    m_cmbAux1Mid->setCurrentIndex(1);
    aux1ModeLayout->addWidget(m_cmbAux1Low);
    aux1ModeLayout->addWidget(m_cmbAux1Mid);
    aux1ModeLayout->addWidget(m_cmbAux1High);
    modeLayout->addLayout(aux1ModeLayout);

    // AUX2
    QHBoxLayout *aux2Layout = new QHBoxLayout();
    aux2Layout->addWidget(new QLabel("AUX2"));
    QSlider *aux2Slider = new QSlider(Qt::Horizontal);
    aux2Slider->setRange(1000, 2000);
    aux2Slider->setValue(1500);
    QSpinBox *aux2Spin = new QSpinBox();
    aux2Spin->setRange(1000, 2000);
    aux2Spin->setValue(1500);
    connect(aux2Slider, &QSlider::valueChanged, aux2Spin, &QSpinBox::setValue);
    connect(aux2Spin, QOverload<int>::of(&QSpinBox::valueChanged), aux2Slider, &QSlider::setValue);
    aux2Layout->addWidget(aux2Slider);
    aux2Layout->addWidget(aux2Spin);
    modeLayout->addLayout(aux2Layout);

    QHBoxLayout *aux2ModeLayout = new QHBoxLayout();
    aux2ModeLayout->addWidget(new QLabel("模式"));
    m_cmbAux2Low = new QComboBox();
    m_cmbAux2Mid = new QComboBox();
    m_cmbAux2High = new QComboBox();
    initFlyModeComboBox(m_cmbAux2Low);
    initFlyModeComboBox(m_cmbAux2Mid);
    initFlyModeComboBox(m_cmbAux2High);
    m_cmbAux2Mid->setCurrentIndex(1);
    aux2ModeLayout->addWidget(m_cmbAux2Low);
    aux2ModeLayout->addWidget(m_cmbAux2Mid);
    aux2ModeLayout->addWidget(m_cmbAux2High);
    modeLayout->addLayout(aux2ModeLayout);

    // AUX3
    QHBoxLayout *aux3Layout = new QHBoxLayout();
    aux3Layout->addWidget(new QLabel("AUX3"));
    QSlider *aux3Slider = new QSlider(Qt::Horizontal);
    aux3Slider->setRange(1000, 2000);
    aux3Slider->setValue(1500);
    QSpinBox *aux3Spin = new QSpinBox();
    aux3Spin->setRange(1000, 2000);
    aux3Spin->setValue(1500);
    connect(aux3Slider, &QSlider::valueChanged, aux3Spin, &QSpinBox::setValue);
    connect(aux3Spin, QOverload<int>::of(&QSpinBox::valueChanged), aux3Slider, &QSlider::setValue);
    aux3Layout->addWidget(aux3Slider);
    aux3Layout->addWidget(aux3Spin);
    modeLayout->addLayout(aux3Layout);

    QHBoxLayout *aux3ModeLayout = new QHBoxLayout();
    aux3ModeLayout->addWidget(new QLabel("模式"));
    m_cmbAux3Low = new QComboBox();
    m_cmbAux3Mid = new QComboBox();
    m_cmbAux3High = new QComboBox();
    initFlyModeComboBox(m_cmbAux3Low);
    initFlyModeComboBox(m_cmbAux3Mid);
    initFlyModeComboBox(m_cmbAux3High);
    m_cmbAux3Mid->setCurrentIndex(1);
    aux3ModeLayout->addWidget(m_cmbAux3Low);
    aux3ModeLayout->addWidget(m_cmbAux3Mid);
    aux3ModeLayout->addWidget(m_cmbAux3High);
    modeLayout->addLayout(aux3ModeLayout);

    // AUX4（仅显示，不参与模式配置）
    QHBoxLayout *aux4Layout = new QHBoxLayout();
    aux4Layout->addWidget(new QLabel("AUX4"));
    QSlider *aux4Slider = new QSlider(Qt::Horizontal);
    aux4Slider->setRange(1000, 2000);
    aux4Slider->setValue(1500);
    QSpinBox *aux4Spin = new QSpinBox();
    aux4Spin->setRange(1000, 2000);
    aux4Spin->setValue(1500);
    connect(aux4Slider, &QSlider::valueChanged, aux4Spin, &QSpinBox::setValue);
    connect(aux4Spin, QOverload<int>::of(&QSpinBox::valueChanged), aux4Slider, &QSlider::setValue);
    aux4Layout->addWidget(aux4Slider);
    aux4Layout->addWidget(aux4Spin);
    modeLayout->addLayout(aux4Layout);

    QHBoxLayout *aux4ModeLayout = new QHBoxLayout();
    aux4ModeLayout->addWidget(new QLabel("模式"));
    QComboBox *aux4Mode1 = new QComboBox();
    QComboBox *aux4Mode2 = new QComboBox();
    QComboBox *aux4Mode3 = new QComboBox();
    aux4Mode1->addItems({"模式1", "模式2", "模式3"});
    aux4Mode2->addItems({"模式1", "模式2", "模式3"});
    aux4Mode2->setCurrentIndex(1);
    aux4Mode3->addItems({"模式1", "模式2", "模式3"});
    aux4ModeLayout->addWidget(aux4Mode1);
    aux4ModeLayout->addWidget(aux4Mode2);
    aux4ModeLayout->addWidget(aux4Mode3);
    modeLayout->addLayout(aux4ModeLayout);

    // 飞行模式读写按钮
    QHBoxLayout *flyModeBtnLayout = new QHBoxLayout();
    QPushButton *btnWriteFlyMode = new QPushButton("写入模式");
    QPushButton *btnReadFlyMode = new QPushButton("读取模式");
    btnWriteFlyMode->setMinimumHeight(40);
    btnReadFlyMode->setMinimumHeight(40);
    connect(btnWriteFlyMode, &QPushButton::clicked, this, &CalibrateTab::on_btnWriteFlyMode_clicked);
    connect(btnReadFlyMode, &QPushButton::clicked, this, &CalibrateTab::on_btnReadFlyMode_clicked);
    flyModeBtnLayout->addWidget(btnWriteFlyMode);
    flyModeBtnLayout->addWidget(btnReadFlyMode);
    modeLayout->addLayout(flyModeBtnLayout);

    topLayout->addWidget(groupMode, 2);

    // ============== 中列：传感器校准区 ==============
    QGroupBox *groupCal = new QGroupBox("传感器校准");
    QVBoxLayout *calMainLayout = new QVBoxLayout(groupCal);
    calMainLayout->setSpacing(20);
    calMainLayout->setContentsMargins(20,20,20,20);

    // 第一行
    QHBoxLayout *calRow1 = new QHBoxLayout();
    QPushButton *btnGyro = new QPushButton("陀螺仪校准");
    QPushButton *btnAcc = new QPushButton("加速度校准");
    QPushButton *btnMag = new QPushButton("罗盘校准");
    btnGyro->setMinimumSize(180, 120);
    btnAcc->setMinimumSize(180, 120);
    btnMag->setMinimumSize(180, 120);
    calRow1->addWidget(btnGyro);
    calRow1->addWidget(btnAcc);
    calRow1->addWidget(btnMag);
    calMainLayout->addLayout(calRow1);

    // 第二行
    QHBoxLayout *calRow2 = new QHBoxLayout();
    QPushButton *btn6Face = new QPushButton("6面校准");
    QPushButton *btnBaro = new QPushButton("气压计校准");
    btn6Face->setMinimumSize(180, 120);
    btnBaro->setMinimumSize(180, 120);
    calRow2->addWidget(btn6Face);
    calRow2->addWidget(btnBaro);
    calMainLayout->addLayout(calRow2);

    // 第三行：光流校准
    QHBoxLayout *calRow3 = new QHBoxLayout();
    QPushButton *btnAltCal = new QPushButton("高度校准(光流)");
    QPushButton *btnInertiaCal = new QPushButton("惯性校准(光流)");
    btnAltCal->setMinimumSize(180, 80);
    btnInertiaCal->setMinimumSize(180, 80);
    calRow3->addWidget(btnAltCal);
    calRow3->addWidget(btnInertiaCal);
    calMainLayout->addLayout(calRow3);

    // 6面校准步骤
    QGroupBox *group6Face = new QGroupBox("6面校准步骤");
    QVBoxLayout *faceLayout = new QVBoxLayout(group6Face);
    QHBoxLayout *faceBtnRow1 = new QHBoxLayout();
    QHBoxLayout *faceBtnRow2 = new QHBoxLayout();

    QPushButton *btnFace1 = new QPushButton("第1步");
    QPushButton *btnFace2 = new QPushButton("第2步");
    QPushButton *btnFace3 = new QPushButton("第3步");
    QPushButton *btnFace4 = new QPushButton("第4步");
    QPushButton *btnFace5 = new QPushButton("第5步");
    QPushButton *btnFace6 = new QPushButton("第6步");
    QPushButton *btnFaceExit = new QPushButton("退出校准");

    btnFace1->setMinimumHeight(40);
    btnFace2->setMinimumHeight(40);
    btnFace3->setMinimumHeight(40);
    btnFace4->setMinimumHeight(40);
    btnFace5->setMinimumHeight(40);
    btnFace6->setMinimumHeight(40);
    btnFaceExit->setMinimumHeight(40);

    faceBtnRow1->addWidget(btnFace1);
    faceBtnRow1->addWidget(btnFace2);
    faceBtnRow1->addWidget(btnFace3);
    faceBtnRow1->addWidget(btnFace4);
    faceBtnRow2->addWidget(btnFace5);
    faceBtnRow2->addWidget(btnFace6);
    faceBtnRow2->addWidget(btnFaceExit);

    faceLayout->addLayout(faceBtnRow1);
    faceLayout->addLayout(faceBtnRow2);
    calMainLayout->addWidget(group6Face);
    topLayout->addWidget(groupCal, 3);

    // ============== 右列：其他参数区 ==============
    QGroupBox *groupParam = new QGroupBox("其他参数");
    QFormLayout *paramLayout = new QFormLayout(groupParam);
    paramLayout->setSpacing(15);
    paramLayout->setContentsMargins(20,20,20,20);

    m_cmbParam = new QComboBox();
    for(const ParamConfig &cfg : PARAM_LIST) {
        m_cmbParam->addItem(cfg.name);
    }
    paramLayout->addRow("参数名称", m_cmbParam);

    m_sbParamId = new QSpinBox();
    m_sbParamId->setRange(0, 65535);
    m_sbParamId->setReadOnly(true);
    paramLayout->addRow("参数序号", m_sbParamId);

    m_sbParamValue = new QDoubleSpinBox();
    m_sbParamValue->setRange(-99999, 99999);
    paramLayout->addRow("参数值", m_sbParamValue);

    m_lbParamDesc = new QLabel();
    m_lbParamDesc->setWordWrap(true);
    m_lbParamDesc->setStyleSheet("color: #666666;");
    paramLayout->addRow("参数说明", m_lbParamDesc);


    QHBoxLayout *btnLayout = new QHBoxLayout();
    QPushButton *btnWrite = new QPushButton("写入");
    connect(btnWrite, &QPushButton::clicked, this, &CalibrateTab::on_btnParamWrite_clicked);
    btnLayout->addWidget(btnWrite);
    paramLayout->addRow(btnLayout);

    topLayout->addWidget(groupParam, 2);

    // 日志区 ---
    QGroupBox *groupLog = new QGroupBox("操作日志");
    QVBoxLayout *logLayout = new QVBoxLayout(groupLog);
    m_tbLog = new QTextEdit();
    m_tbLog->setReadOnly(true);
    QPushButton *btnClearLog = new QPushButton("清空日志");
    connect(btnClearLog, &QPushButton::clicked, this, [=](){ m_tbLog->clear(); });

    logLayout->addWidget(m_tbLog);
    logLayout->addWidget(btnClearLog);

    mainLayout->addLayout(topLayout);
    mainLayout->addWidget(groupLog);
    mainLayout->setStretch(0, 7);
    mainLayout->setStretch(1, 3);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(10,10,10,10);
    this->setLayout(mainLayout);

    // ====================== 信号连接 ======================
    connect(btnGyro, &QPushButton::clicked, this, &CalibrateTab::on_btnGyroCal_clicked);
    connect(btnAcc, &QPushButton::clicked, this, &CalibrateTab::on_btnAccCal_clicked);
    connect(btnMag, &QPushButton::clicked, this, &CalibrateTab::on_btnMagCal_clicked);
    connect(btnBaro, &QPushButton::clicked, this, &CalibrateTab::on_btnBaroCal_clicked);
    connect(btnAltCal, &QPushButton::clicked, this, &CalibrateTab::on_btnAltCal_clicked);
    connect(btnInertiaCal, &QPushButton::clicked, this, &CalibrateTab::on_btnInertiaCal_clicked);
    connect(btnFace1, &QPushButton::clicked, this, &CalibrateTab::on_btn6FaceCalStep1_clicked);
    connect(btnFace2, &QPushButton::clicked, this, &CalibrateTab::on_btn6FaceCalStep2_clicked);
    connect(btnFace3, &QPushButton::clicked, this, &CalibrateTab::on_btn6FaceCalStep3_clicked);
    connect(btnFace4, &QPushButton::clicked, this, &CalibrateTab::on_btn6FaceCalStep4_clicked);
    connect(btnFace5, &QPushButton::clicked, this, &CalibrateTab::on_btn6FaceCalStep5_clicked);
    connect(btnFace6, &QPushButton::clicked, this, &CalibrateTab::on_btn6FaceCalStep6_clicked);
    connect(btnFaceExit, &QPushButton::clicked, this, &CalibrateTab::on_btn6FaceCalExit_clicked);

    on_cmbParam_currentIndexChanged(0);
    connect(m_cmbParam, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CalibrateTab::on_cmbParam_currentIndexChanged);
}



// ====================== 下拉框初始化 ======================
void CalibrateTab::initFlyModeComboBox(QComboBox *cmb)
{
    cmb->addItem("模式1", 1);
    cmb->addItem("模式2", 2);
    cmb->addItem("模式3", 3);
    cmb->addItem("模式4", 4);
    cmb->addItem("模式5", 5);
    cmb->addItem("模式6", 6);
}

void CalibrateTab::on_cmbParam_currentIndexChanged(int index)
{
    if(index < 0 || index >= PARAM_LIST.size()) return;
    const ParamConfig &cfg = PARAM_LIST[index];
    m_sbParamId->setValue(cfg.id);
    m_lbParamDesc->setText(cfg.desc);
    m_sbParamValue->setSuffix(" " + cfg.unit);
}

void CalibrateTab::showMsg(const QString &msg)
{
    m_tbLog->append(QString("[%1] %2").arg(QTime::currentTime().toString("hh:mm:ss")).arg(msg));
}

uint8_t CalibrateTab::calcChecksum(uint8_t *data, int len)
{
    uint8_t sum = 0;
    for (int i = 0; i < len; i++) sum += data[i];
    return sum & 0xFF;
}

// ====================== 发送校准命令 ======================
void CalibrateTab::sendCalibrateCmd(uint8_t cmd)
{
    if (!m_serialThread) {
        QMessageBox::warning(this, "错误", "串口未初始化！");
        return;
    }

    QByteArray frame;
    frame.append(TX_HEAD1);
    frame.append(TX_HEAD2);
    frame.append(FUNC_CALIBRATE);
    frame.append((uint8_t)0x01);
    frame.append(cmd);
    frame.append(calcChecksum((uint8_t*)frame.constData(), frame.size()));

    m_serialThread->sendData(frame);
    qDebug() << "[校准] 发送命令帧:" << frame.toHex(' ').toUpper();
}

// ====================== 校准按钮点击======================
void CalibrateTab::on_btnGyroCal_clicked()
{
    sendCalibrateCmd(0x02);
    showMsg("已发送 → 陀螺仪校准命令");
}

void CalibrateTab::on_btnAccCal_clicked()
{
    sendCalibrateCmd(0x01);
    showMsg("已发送 → 加速度校准命令");
}

void CalibrateTab::on_btnMagCal_clicked()
{
    sendCalibrateCmd(0x04);
    showMsg("已发送 → 罗盘校准命令");
}

void CalibrateTab::on_btnBaroCal_clicked()
{
    sendCalibrateCmd(0x05);
    showMsg("已发送 → 气压计校准命令");
}

void CalibrateTab::on_btnAltCal_clicked()
{
    sendCalibrateCmd(0x03);
    showMsg("已发送 → 光流高度校准命令");
}

void CalibrateTab::on_btnInertiaCal_clicked()
{
    sendCalibrateCmd(0x10);
    showMsg("已发送 → 光流惯性校准命令");
}

void CalibrateTab::on_btn6FaceCalStep1_clicked() {
    sendCalibrateCmd(0x21);
    showMsg("已发送 → 6面校准第1步");
}
void CalibrateTab::on_btn6FaceCalStep2_clicked() {
    sendCalibrateCmd(0x22);
    showMsg("已发送 → 6面校准第2步");
}
void CalibrateTab::on_btn6FaceCalStep3_clicked() {
    sendCalibrateCmd(0x23);
    showMsg("已发送 → 6面校准第3步");
}
void CalibrateTab::on_btn6FaceCalStep4_clicked() {
    sendCalibrateCmd(0x24);
    showMsg("已发送 → 6面校准第4步");
}
void CalibrateTab::on_btn6FaceCalStep5_clicked() {
    sendCalibrateCmd(0x25);
    showMsg("已发送 → 6面校准第5步");
}
void CalibrateTab::on_btn6FaceCalStep6_clicked() {
    sendCalibrateCmd(0x26);
    showMsg("已发送 → 6面校准第6步");
}
void CalibrateTab::on_btn6FaceCalExit_clicked() {
    sendCalibrateCmd(0x20);
    showMsg("已发送 → 退出6面校准");
}

// ====================== 参数设置（大端模式）======================
void CalibrateTab::sendParamSetCmd(uint16_t paramId, uint32_t value)
{
    if (!m_serialThread) {
        QMessageBox::warning(this, "错误", "串口未初始化！");
        return;
    }

    QByteArray frame;
    frame.append(TX_HEAD1);
    frame.append(TX_HEAD2);
    frame.append(FUNC_PARAM_SET);
    frame.append((uint8_t)0x06); // 长度6字节

    // 【大端模式】参数ID（2字节，高字节在前）
    frame.append((uint8_t)((paramId >> 8) & 0xFF));
    frame.append((uint8_t)(paramId & 0xFF));

    // 【大端模式】参数值（4字节，高字节在前）
    frame.append((uint8_t)((value >> 24) & 0xFF));
    frame.append((uint8_t)((value >> 16) & 0xFF));
    frame.append((uint8_t)((value >> 8) & 0xFF));
    frame.append((uint8_t)(value & 0xFF));

    frame.append(calcChecksum((uint8_t*)frame.constData(), frame.size()));

    m_serialThread->sendData(frame);
    qDebug() << "[参数] 发送设置帧:" << frame.toHex(' ').toUpper();
}

void CalibrateTab::on_btnParamWrite_clicked()
{
    int index = m_cmbParam->currentIndex();
    if(index < 0 || index >= PARAM_LIST.size()) return;

    const ParamConfig &cfg = PARAM_LIST[index];
    float inputValue = m_sbParamValue->value();
    uint32_t sendValue = (uint32_t)(inputValue * cfg.scale);

    sendParamSetCmd(cfg.id, sendValue);
    showMsg(QString("已发送 → 写入%1：%2 %3").arg(cfg.name).arg(inputValue).arg(cfg.unit));
}

// ====================== 飞行模式写入（纯发送，无等待）======================
void CalibrateTab::on_btnWriteFlyMode_clicked()
{
    if (!m_serialThread || !m_serialThread->isOpen()) {
        QMessageBox::warning(this, "错误", "串口未连接！");
        return;
    }

    // 读取所有下拉框的值
    int8_t aux1_low = m_cmbAux1Low->currentData().toInt();
    int8_t aux1_mid = m_cmbAux1Mid->currentData().toInt();
    int8_t aux1_high = m_cmbAux1High->currentData().toInt();

    int8_t aux2_low = m_cmbAux2Low->currentData().toInt();
    int8_t aux2_mid = m_cmbAux2Mid->currentData().toInt();
    int8_t aux2_high = m_cmbAux2High->currentData().toInt();

    int8_t aux3_low = m_cmbAux3Low->currentData().toInt();
    int8_t aux3_mid = m_cmbAux3Mid->currentData().toInt();
    int8_t aux3_high = m_cmbAux3High->currentData().toInt();

    // 按协议打包帧
    QByteArray frame;
    frame.append(TX_HEAD1);
    frame.append(TX_HEAD2);
    frame.append(FUNC_FLY_MODE);
    frame.append((uint8_t)0x09); // 固定9字节载荷

    frame.append((uint8_t)aux1_low);
    frame.append((uint8_t)aux1_mid);
    frame.append((uint8_t)aux1_high);
    frame.append((uint8_t)aux2_low);
    frame.append((uint8_t)aux2_mid);
    frame.append((uint8_t)aux2_high);
    frame.append((uint8_t)aux3_low);
    frame.append((uint8_t)aux3_mid);
    frame.append((uint8_t)aux3_high);

    frame.append(calcChecksum((uint8_t*)frame.constData(), frame.size()));

    // 纯发送，不等待
    m_serialThread->sendData(frame);
    qDebug() << "[飞行模式] 发送配置帧:" << frame.toHex(' ').toUpper();
    showMsg("已发送 → 飞行模式配置");
}

// 读取飞行模式（保留，因为需要接收数据）
void CalibrateTab::on_btnReadFlyMode_clicked()
{
    sendReadFlyModeRequest();
    showMsg("已发送读取飞行模式请求");
}

void CalibrateTab::sendReadFlyModeRequest()
{
    if (!m_serialThread) {
        QMessageBox::warning(this, "错误", "串口未初始化！");
        return;
    }

    QByteArray frame;
    frame.append(TX_HEAD1);
    frame.append(TX_HEAD2);
    frame.append(FUNC_ACK);
    frame.append((uint8_t)0x01);
    frame.append((uint8_t)0x02);

    uint8_t sum = 0;
    for (int i = 0; i < frame.size(); i++) {
        sum += (uint8_t)frame[i];
    }
    frame.append(sum);

    m_serialThread->sendData(frame);
    qDebug() << "[飞行模式] 发送读取请求帧:" << frame.toHex(' ').toUpper();
}

// ====================== 统一数据解析：飞行模式 + 参数上报 ======================
void CalibrateTab::parseFlyModeData(QByteArray frame)
{
    // 1. 通用帧校验（所有帧共用）
    if (frame.size() < 6) return; // 最小帧长度校验

    uint8_t h1 = (uint8_t)frame[0];
    uint8_t h2 = (uint8_t)frame[1];
    if (h1 != RX_HEAD1 || h2 != RX_HEAD2) return; // 帧头校验

    uint8_t func    = (uint8_t)frame[2];
    uint8_t payLen  = (uint8_t)frame[3];
    int totalLen    = 4 + payLen + 1;

    if (frame.size() != totalLen) return; // 帧长度校验

    // 全帧累加校验
    uint8_t sum = 0;
    for (int i=0; i<totalLen-1; i++) sum += (uint8_t)frame[i];
    if (sum != (uint8_t)frame[totalLen-1]) return; // 校验和校验

    // ==============================================
    // 分支1：处理飞行模式数据帧（功能码0x0A，原有逻辑完全保留）
    // ==============================================
    if (func == FUNC_FLY_MODE && payLen == 9)
    {
        qDebug() << "[飞行模式] 收到正确数据帧:" << frame.toHex(' ').toUpper();

        uint8_t aux1_low  = (uint8_t)frame[4];
        uint8_t aux1_mid  = (uint8_t)frame[5];
        uint8_t aux1_high = (uint8_t)frame[6];
        uint8_t aux2_low  = (uint8_t)frame[7];
        uint8_t aux2_mid  = (uint8_t)frame[8];
        uint8_t aux2_high = (uint8_t)frame[9];
        uint8_t aux3_low  = (uint8_t)frame[10];
        uint8_t aux3_mid  = (uint8_t)frame[11];
        uint8_t aux3_high = (uint8_t)frame[12];

        m_cmbAux1Low->setCurrentIndex(m_cmbAux1Low->findData(aux1_low));
        m_cmbAux1Mid->setCurrentIndex(m_cmbAux1Mid->findData(aux1_mid));
        m_cmbAux1High->setCurrentIndex(m_cmbAux1High->findData(aux1_high));

        m_cmbAux2Low->setCurrentIndex(m_cmbAux2Low->findData(aux2_low));
        m_cmbAux2Mid->setCurrentIndex(m_cmbAux2Mid->findData(aux2_mid));
        m_cmbAux2High->setCurrentIndex(m_cmbAux2High->findData(aux2_high));

        m_cmbAux3Low->setCurrentIndex(m_cmbAux3Low->findData(aux3_low));
        m_cmbAux3Mid->setCurrentIndex(m_cmbAux3Mid->findData(aux3_mid));
        m_cmbAux3High->setCurrentIndex(m_cmbAux3High->findData(aux3_high));

        showMsg("✅ 飞行模式读取成功，已自动填充到界面");
        return;
    }

    // 分支2处理PARAMETER SET参数上报帧（功能码0x09，大端模式）
    // ==============================================
    if (func == FUNC_PARAM_SET && payLen == 6) // 功能码0x09，长度6字节（2+4）
    {
        qDebug() << "[参数] 收到参数上报帧:" << frame.toHex(' ').toUpper();

        // 【大端模式解析】：先高字节，后低字节
        // 1. 解析2字节参数ID（大端）
        uint16_t paramId = ( static_cast<uint16_t>(frame[4]) << 8 ) | static_cast<uint16_t>(frame[5]);
        // 2. 解析4字节参数值（大端）
        uint32_t rawValue = ( static_cast<uint32_t>(frame[6]) << 24 ) |
                            ( static_cast<uint32_t>(frame[7]) << 16 ) |
                            ( static_cast<uint32_t>(frame[8]) << 8  ) |
                            static_cast<uint32_t>(frame[9]);

        // 3. 遍历参数配置表，匹配参数ID
        for (int i=0; i<PARAM_LIST.size(); i++)
        {
            const ParamConfig &cfg = PARAM_LIST[i];
            if (cfg.id == paramId)
            {
                // 4. 按缩放系数还原物理值：rawValue = 物理值 * scale → 物理值 = rawValue / scale
                float physicalValue = static_cast<float>(rawValue) / cfg.scale;

                // 5. 自动切换到该参数，并填充数值
                m_cmbParam->setCurrentIndex(i);
                m_sbParamValue->setValue(physicalValue);

                showMsg(QString("✅ 收到%1上报，值：%2 %3")
                            .arg(cfg.name)
                            .arg(physicalValue, 0, 'f', 2)
                            .arg(cfg.unit));
                return;
            }
        }

        // 未匹配到参数ID的情况
        showMsg(QString("⚠️ 收到未知参数ID：0x%1，原始值：%2")
                    .arg(QString::number(paramId, 16).toUpper())
                    .arg(rawValue));
        return;
    }
}