#ifndef CALIBRATETAB_H
#define CALIBRATETAB_H

#include <QWidget>
#include <QGroupBox>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFormLayout>
#include <QSlider>
#include "SerialThread.h"

class CalibrateTab : public QWidget
{
    Q_OBJECT

public:

    explicit CalibrateTab(SerialThread *serialThread = nullptr, QWidget *parent = nullptr);
    ~CalibrateTab();
    void setSerialThread(SerialThread* thread);

private slots:
    // 传感器校准按钮槽函数
    void on_btnGyroCal_clicked();
    void on_btnAccCal_clicked();
    void on_btnMagCal_clicked();
    void on_btnBaroCal_clicked();
    void on_btnAltCal_clicked();
    void on_btnInertiaCal_clicked();

    // 6面校准步骤槽函数
    void on_btn6FaceCalStep1_clicked();
    void on_btn6FaceCalStep2_clicked();
    void on_btn6FaceCalStep3_clicked();
    void on_btn6FaceCalStep4_clicked();
    void on_btn6FaceCalStep5_clicked();
    void on_btn6FaceCalStep6_clicked();
    void on_btn6FaceCalExit_clicked();

    // 参数设置槽函数
    void on_cmbParam_currentIndexChanged(int index);
    void on_btnParamWrite_clicked();

    // 飞行模式槽函数
    void on_btnWriteFlyMode_clicked();
    void on_btnReadFlyMode_clicked();

    // 统一数据解析（飞行模式 + 参数上报）
    void parseFlyModeData(QByteArray frame);

private:
    SerialThread *m_serialThread;

    // UI控件
    QTextEdit *m_tbLog;
    QComboBox *m_cmbParam;
    QSpinBox *m_sbParamId;
    QDoubleSpinBox *m_sbParamValue;
    QLabel *m_lbParamDesc;

    // 飞行模式控件
    QComboBox *m_cmbAux1Low, *m_cmbAux1Mid, *m_cmbAux1High;
    QComboBox *m_cmbAux2Low, *m_cmbAux2Mid, *m_cmbAux2High;
    QComboBox *m_cmbAux3Low, *m_cmbAux3Mid, *m_cmbAux3High;

    // 内部辅助函数
    void initUI();
    void showMsg(const QString &msg);
    uint8_t calcChecksum(uint8_t *data, int len);
    void sendCalibrateCmd(uint8_t cmd); // 纯发送，无等待
    void sendParamSetCmd(uint16_t paramId, uint32_t value);
    void initFlyModeComboBox(QComboBox *cmb);
    void sendReadFlyModeRequest();

    // ================== 协议常量 ==================
    static const uint8_t TX_HEAD1 = 0xAA;
    static const uint8_t TX_HEAD2 = 0xAF;
    static const uint8_t RX_HEAD1 = 0xAA;
    static const uint8_t RX_HEAD2 = 0xAA;
    static const uint8_t FUNC_CALIBRATE = 0x01;
    static const uint8_t FUNC_PARAM_SET = 0x09;
    static const uint8_t FUNC_FLY_MODE = 0x0A;
    static const uint8_t FUNC_ACK = 0x02;
};

#endif // CALIBRATETAB_H