#include "AttitudeWaveChart.h"
#include <QVBoxLayout>

AttitudeWaveChart::AttitudeWaveChart(QWidget *parent)
    : QWidget(parent), m_pointCount(0)
{
    initChart();
}

void AttitudeWaveChart::initChart()
{
    // 初始化曲线
    m_seriesRoll = new QtCharts::QLineSeries;
    m_seriesPitch = new QtCharts::QLineSeries;
    m_seriesYaw = new QtCharts::QLineSeries;

    m_seriesRoll->setName("ROLL");
    m_seriesPitch->setName("PITCH");
    m_seriesYaw->setName("YAW");
    m_seriesRoll->setColor(QColor("#1f77b4"));
    m_seriesPitch->setColor(QColor("#2ca02c"));
    m_seriesYaw->setColor(QColor("#ff7f0e"));

    // 初始化图表
    m_chart = new QtCharts::QChart;
    m_chart->legend()->setVisible(true);
    m_chart->setTitle("姿态角波形");
    m_chart->addSeries(m_seriesRoll);
    m_chart->addSeries(m_seriesPitch);
    m_chart->addSeries(m_seriesYaw);

    // 坐标轴
    m_axisX = new QtCharts::QValueAxis;
    m_axisY = new QtCharts::QValueAxis;
    m_axisX->setRange(0, 200);
    m_axisY->setRange(-180, 180);
    m_axisX->setTitleText("采样点");
    m_axisY->setTitleText("角度(°)");

    m_chart->addAxis(m_axisX, Qt::AlignBottom);
    m_chart->addAxis(m_axisY, Qt::AlignLeft);
    m_seriesRoll->attachAxis(m_axisX);
    m_seriesRoll->attachAxis(m_axisY);
    m_seriesPitch->attachAxis(m_axisX);
    m_seriesPitch->attachAxis(m_axisY);
    m_seriesYaw->attachAxis(m_axisX);
    m_seriesYaw->attachAxis(m_axisY);

    // 图表视图
    m_chartView = new QtCharts::QChartView(m_chart);
    m_chartView->setRenderHint(QPainter::Antialiasing);

    // 布局
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(m_chartView);
    layout->setContentsMargins(0,0,0,0);
}

void AttitudeWaveChart::updateData(float roll, float pitch, float yaw)
{
    m_seriesRoll->append(m_pointCount, roll);
    m_seriesPitch->append(m_pointCount, pitch);
    m_seriesYaw->append(m_pointCount, yaw);
    m_pointCount++;

    // 滚动显示
    if(m_pointCount > 200) {
        m_seriesRoll->removePoints(0, 1);
        m_seriesPitch->removePoints(0, 1);
        m_seriesYaw->removePoints(0, 1);
        m_axisX->setRange(m_pointCount - 200, m_pointCount);
    }
}
