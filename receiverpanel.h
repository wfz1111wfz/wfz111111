#ifndef RECEIVERPANEL_H
#define RECEIVERPANEL_H

#include <QWidget>
#include <QGroupBox>
#include <QFormLayout>
#include <QLabel>
#include <QVBoxLayout>

class ReceiverPanel : public QWidget
{
    Q_OBJECT
public:
    explicit ReceiverPanel(QWidget *parent = nullptr);

public slots:
    // 改成槽函数，方便信号连接
    void updateChannelData(int thr, int yaw, int rol, int pit, int aux1, int aux2, int aux3, int aux4, int aux5, int aux6);

private:
    void initUI();

    // 通道标签
    QLabel *m_lbThr;
    QLabel *m_lbYaw;
    QLabel *m_lbRol;
    QLabel *m_lbPit;
    QLabel *m_lbAux1;
    QLabel *m_lbAux2;
    QLabel *m_lbAux3;
    QLabel *m_lbAux4;
    QLabel *m_lbAux5;
    QLabel *m_lbAux6;
};

#endif // RECEIVERPANEL_H
