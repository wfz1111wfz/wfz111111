#ifndef GPSINFOPANEL_H
#define GPSINFOPANEL_H

#include <QWidget>
#include <QGroupBox>
#include <QFormLayout>
#include <QLabel>

class GpsInfoPanel : public QWidget
{
    Q_OBJECT
public:
    explicit GpsInfoPanel(QWidget *parent = nullptr);
    void updateGpsData(double lon, double lat, int fixType, int satsNum, float alt, float speedN, float speedE, float yaw);

private:
    void initUI();

    QLabel *m_lbLon;
    QLabel *m_lbLat;
    QLabel *m_lbFix;
    QLabel *m_lbSats;
    QLabel *m_lbAlt;
    QLabel *m_lbSpeedN;
    QLabel *m_lbSpeedE;
    QLabel *m_lbYaw;
};

#endif // GPSINFOPANEL_H
