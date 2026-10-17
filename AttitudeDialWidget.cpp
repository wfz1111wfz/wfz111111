#include "AttitudeDialWidget.h"
#include <QPainter>
#include <QFont>
#include <QPen>
#include <QBrush>


AttitudeDialWidget::AttitudeDialWidget(QWidget *parent)
    : QWidget(parent), m_roll(0), m_pitch(0), m_yaw(0)
{
    // 连接定时器信号：每30ms触发一次重绘（约33fps，保证动画流畅不卡顿）
    connect(&m_updateTimer, &QTimer::timeout, this, QOverload<>::of(&QWidget::update));
    m_updateTimer.start(30); // 启动定时器，间隔30毫秒
}

void AttitudeDialWidget::setAttitude(float roll, float pitch, float yaw)
{
    // 更新内部存储的姿态数据
    m_roll = roll;
    m_pitch = pitch;
    m_yaw = yaw;

    // 触发重绘事件（异步调用 paintEvent）
    update();
}


void AttitudeDialWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event); // 未使用 event 参数，避免编译警告

    // -------------------------- 初始化绘制环境 --------------------------
    QPainter painter(this);
    // 开启抗锯齿，让线条和文字更平滑
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::TextAntialiasing);

    // 计算基础几何参数
    int w = width();                          // 控件宽度
    int h = height();                         // 控件高度
    int centerX = w / 2;                      // 控件中心X坐标
    int centerY = h / 2;                      // 控件中心Y坐标
    int radius = qMin(w, h) / 2 - 10;        // 地平仪半径（取宽高中较小值，留10px边距）
    const float pitchPixelPerDegree = 3.0f;   // 俯仰角像素比例：1度对应3个像素

    // -------------------------- 设置圆形绘制遮罩 --------------------------
    painter.save(); // 保存当前绘制状态（用于后续恢复）

    // 创建圆形路径作为遮罩
    QPainterPath maskPath;
    maskPath.addEllipse(QRect(-radius, -radius, radius*2, radius*2));

    // 将坐标系原点移到控件中心（方便后续绘制）
    painter.translate(centerX, centerY);
    // 应用圆形遮罩：只显示圆形区域内的内容
    painter.setClipPath(maskPath);

    // -------------------------- 绘制地平仪核心区域 --------------------------
    painter.save(); // 保存状态

    // 应用横滚角旋转：整个地平仪绕中心旋转 Roll 度
    painter.rotate(-m_roll);

    // 计算俯仰偏移量：Pitch 每变化1度，地平仪上下移动3个像素
    float pitchOffset = m_pitch * pitchPixelPerDegree;
    // 应用俯仰平移：地平仪沿Y轴上下移动
    painter.translate(0, pitchOffset);

    // 定义绘制范围（足够大的矩形，保证旋转后仍能覆盖整个圆形）
    int drawRange = radius * 4;

    // 绘制天空（上半部分，蓝色）
    painter.setPen(Qt::NoPen);        // 不绘制边框
    painter.setBrush(QColor(QRgb(0x87CEFA))); // 设置天空颜色（淡蓝色）
    painter.drawRect(-drawRange, -drawRange, drawRange*2, drawRange);

    // 绘制地面（下半部分，绿色）
    painter.setBrush(QColor(QRgb(0x228B22))); // 设置地面颜色（森林绿）
    painter.drawRect(-drawRange, 0, drawRange*2, drawRange);

    // 绘制地平线（天空与地面的分界线，白色粗线）
    QPen horizonPen(QColor(QRgb(0xFFFFFF)), 3); // 白色画笔，线宽3px
    painter.setPen(horizonPen);
    painter.drawLine(-drawRange, 0, drawRange, 0); // 从左到右画一条水平线

    // 3.4 绘制俯仰刻度线和数值
    QPen pitchPen(QColor(QRgb(0xFFFFFF)), 2); // 白色画笔，线宽2px
    painter.setPen(pitchPen);
    QFont pitchFont("Arial", 9, QFont::Bold); // 设置刻度字体：Arial，9号，加粗
    painter.setFont(pitchFont);

    // 循环绘制从-90度到+90度的刻度，步长10度
    for (int i = -90; i <= 90; i += 10) {
        if (i == 0) continue; // 跳过0度（地平线已经画过了）

        // 计算当前刻度的Y坐标（注意负号：因为Qt坐标系Y轴向下）
        float y = -i * pitchPixelPerDegree;

        // 确定刻度线长度：20度的整数倍用长线（25px），其他用短线（15px）
        int lineLen = (i % 20 == 0) ? 25 : 15;

        // 绘制左右对称的刻度线
        painter.drawLine(-radius, y, -radius + lineLen, y); // 左侧刻度线
        painter.drawLine(radius - lineLen, y, radius, y);     // 右侧刻度线

        // 如果是20度的整数倍，绘制数值标签
        if (i % 20 == 0) {
            QString numStr = QString::number(qAbs(i)); // 取绝对值显示
            // 在左侧刻度线右边绘制数值
            painter.drawText(-radius + lineLen + 5, y - 6, numStr);
            // 在右侧刻度线左边绘制数值
            painter.drawText(radius - lineLen - 25, y - 6, numStr);
        }
    }

    painter.restore(); // 恢复状态，结束地平仪核心绘制

    // -------------------------- 绘制中心飞机符号 --------------------------
    painter.save();

    // 设置飞机符号画笔：金色，线宽4px
    QPen planePen(QColor(QRgb(0xFFD700)), 4);
    painter.setPen(planePen);

    // 绘制飞机十字线
    painter.drawLine(-35, 0, 35, 0); // 横线（代表机翼）
    painter.drawLine(0, -8, 0, 8);     // 竖线（代表机身）
    // 绘制中心圆点
    painter.drawEllipse(QPoint(0, 0), 4, 4);

    painter.restore();

    // -------------------------- 绘制外圈航向罗盘 --------------------------
    painter.restore(); // 恢复到最初的状态
    painter.translate(centerX, centerY); // 再次将原点移到中心

    painter.save();
    int headingRadius = radius - 15; // 航向罗盘半径（比地平仪小15px）

    // 应用航向角旋转
    painter.rotate(m_yaw);

    // 绘制航向刻度线
    QPen headingPen(QColor(QRgb(0x000000)), 2); // 黑色画笔，线宽2px
    painter.setPen(headingPen);

    // 循环绘制360度刻度，步长5度
    for (int i = 0; i < 360; i += 5) {
        // 确定刻度线长度：30度的整数倍用长线（20px），其他用短线（10px）
        int len = (i % 30 == 0) ? 20 : 10;
        // 从顶部开始画刻度线
        painter.drawLine(0, -headingRadius, 0, -headingRadius + len);
        // 旋转坐标系5度，准备画下一个刻度
        painter.rotate(5);
    }
    painter.rotate(-360); // 旋转回初始角度

    //绘制方位文字（N/E/S/W）
    QFont headingFont("Arial", 12, QFont::Bold); // 设置方位字体
    painter.setFont(headingFont);
    painter.setPen(QColor(QRgb(0x000000))); // 黑色文字

    // 绘制北(N)
    painter.drawText(-15, -headingRadius + 25, "N");
    painter.rotate(90);
    // 绘制东(E)
    painter.drawText(-15, -headingRadius + 25, "E");
    painter.rotate(90);
    // 绘制南(S)
    painter.drawText(-15, -headingRadius + 25, "S");
    painter.rotate(90);
    // 绘制西(W)
    painter.drawText(-15, -headingRadius + 25, "W");
    painter.rotate(90); // 旋转回初始角度

    painter.restore();

    // -------------------------- 绘制顶部航向指针（红色三角形） --------------------------
    painter.save();
    painter.setBrush(QColor(QRgb(0xFF0000))); // 红色填充
    painter.setPen(Qt::NoPen);                  // 无边框

    // 绘制三角形指针路径
    QPainterPath pointerPath;
    pointerPath.moveTo(0, -radius);               // 三角形顶点（指向正上方）
    pointerPath.lineTo(-10, -radius + 15);       // 左下角
    pointerPath.lineTo(10, -radius + 15);        // 右下角
    pointerPath.closeSubpath();                   // 闭合路径

    painter.drawPath(pointerPath);
    painter.restore();

    // -------------------------- 绘制底部角度数值显示 --------------------------
    painter.save();
    QFont textFont("Arial", 11, QFont::Bold); // 设置数值字体
    painter.setFont(textFont);
    painter.setPen(QColor(QRgb(0x000000))); // 黑色文字

    // 格式化角度字符串：保留1位小数
    QString infoText = QString("Roll: %1°  Pitch: %2°  Yaw: %3°")
                           .arg(m_roll, 0, 'f', 1)
                           .arg(m_pitch, 0, 'f', 1)
                           .arg(m_yaw, 0, 'f', 1);

    // 在底部绘制文字
    painter.drawText(-radius, radius + 10, infoText);
    painter.restore();
}