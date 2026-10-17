#ifndef ATTITUDEDIALWIDGET_H
#define ATTITUDEDIALWIDGET_H

#include <QWidget>
#include <QTimer>
#include <QPainter>
#include <QPaintEvent>
#include <QPainterPath>
#include <QtMath>


class AttitudeDialWidget : public QWidget
{
    Q_OBJECT
public:

    explicit AttitudeDialWidget(QWidget *parent = nullptr);

    void setAttitude(float roll, float pitch, float yaw);

protected:

    void paintEvent(QPaintEvent *event) override;

private:
    // 姿态数据成员变量
    float m_roll;   ///< 横滚角（Roll），单位：度
    float m_pitch;  ///< 俯仰角（Pitch），单位：度
    float m_yaw;    ///< 航向角（Yaw），单位：度

    // 定时刷新器
    QTimer m_updateTimer; ///< 定时器，用于定时触发重绘，保证动画流畅
};

#endif // ATTITUDEDIALWIDGET_H


