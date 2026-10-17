#ifndef DRONE3DVIEW_H
#define DRONE3DVIEW_H

#include <QWidget>
#include <Qt3DExtras/Qt3DWindow>
#include <Qt3DCore/QEntity>
#include <Qt3DCore/QTransform>

class Drone3DView : public QWidget
{
    Q_OBJECT
public:
    explicit Drone3DView(QWidget *parent = nullptr);
    void setAttitude(float roll, float pitch, float yaw);

private:
    void initModel();

    Qt3DExtras::Qt3DWindow *m_3dWindow;
    Qt3DCore::QEntity *m_rootEntity;
    Qt3DCore::QEntity *m_droneEntity;
    Qt3DCore::QTransform *m_droneTransform;
};

#endif // DRONE3DVIEW_H
