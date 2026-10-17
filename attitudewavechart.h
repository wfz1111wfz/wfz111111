#ifndef ATTITUDEWAVECHART_H
#define ATTITUDEWAVECHART_H

#include <QWidget>
#include <QtCharts/QChart>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QtCharts/QChartView>

class AttitudeWaveChart : public QWidget
{
    Q_OBJECT
public:
    explicit AttitudeWaveChart(QWidget *parent = nullptr);
    void updateData(float roll, float pitch, float yaw);

private:
    void initChart();

    QtCharts::QChart* m_chart;
    QtCharts::QLineSeries* m_seriesRoll;
    QtCharts::QLineSeries* m_seriesPitch;
    QtCharts::QLineSeries* m_seriesYaw;
    QtCharts::QValueAxis* m_axisX;
    QtCharts::QValueAxis* m_axisY;
    QtCharts::QChartView* m_chartView;
    int m_pointCount;
};

#endif // ATTITUDEWAVECHART_H
