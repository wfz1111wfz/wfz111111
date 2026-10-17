#include "Drone3DView.h"
#include <QVBoxLayout>
#include <Qt3DExtras/QPhongMaterial>
#include <Qt3DExtras/QCuboidMesh>
#include <Qt3DExtras/QCylinderMesh>
#include <Qt3DRender/QCamera>
#include <Qt3DRender/QPointLight>

Drone3DView::Drone3DView(QWidget *parent)
    : QWidget(parent)
{
    initModel();
}

void Drone3DView::initModel()
{
    // 创建 Qt3D 窗口，并嵌入到当前 QWidget 中
    m_3dWindow = new Qt3DExtras::Qt3DWindow();
    QWidget *container = QWidget::createWindowContainer(m_3dWindow, this);
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(container);
    layout->setContentsMargins(0,0,0,0);
    // 创建根实体
    m_rootEntity = new Qt3DCore::QEntity();
    m_3dWindow->setRootEntity(m_rootEntity);

    // 相机设置
    Qt3DRender::QCamera *camera = m_3dWindow->camera();
    camera->lens()->setPerspectiveProjection(45.0f, 16.0f/9.0f, 0.1f, 1000.0f);//45° 视场角，16:9 比例
    camera->setPosition(QVector3D(0, -8, 5)); // 相机位置：在无人机后方、偏上
    camera->setViewCenter(QVector3D(0, 0, 0));//相机视角中心
    camera->setUpVector(QVector3D(0, 0, 1));// 相机的上方，Z轴向上

    // 主光源
    Qt3DCore::QEntity *mainLight = new Qt3DCore::QEntity(m_rootEntity);
    Qt3DRender::QPointLight *mainLightComp = new Qt3DRender::QPointLight(mainLight);
    mainLightComp->setColor("white");
    mainLightComp->setIntensity(1.8f);
    mainLight->addComponent(mainLightComp);
    Qt3DCore::QTransform *mainLightT = new Qt3DCore::QTransform(mainLight);
    mainLightT->setTranslation(QVector3D(0, -10, 8));
    mainLight->addComponent(mainLightT);

    // 顶部补光
    Qt3DCore::QEntity *topLight = new Qt3DCore::QEntity(m_rootEntity);
    Qt3DRender::QPointLight *topLightComp = new Qt3DRender::QPointLight(topLight);
    topLightComp->setColor("white");
    topLightComp->setIntensity(1.2f);
    topLight->addComponent(topLightComp);
    Qt3DCore::QTransform *topLightT = new Qt3DCore::QTransform(topLight);
    topLightT->setTranslation(QVector3D(0, 0, 10));
    topLight->addComponent(topLightT);

    // 飞机实体，创建无人机父实体
    m_droneEntity = new Qt3DCore::QEntity(m_rootEntity);
    // 创建无人机的变换组件负责旋转
    m_droneTransform = new Qt3DCore::QTransform(m_droneEntity);
    m_droneEntity->addComponent(m_droneTransform);

    // 材质定义
    Qt3DExtras::QPhongMaterial *bodyTopMat = new Qt3DExtras::QPhongMaterial();
    bodyTopMat->setDiffuse(QColor(QRgb(0x333333)));
    Qt3DExtras::QPhongMaterial *bodyBottomMat = new Qt3DExtras::QPhongMaterial();
    bodyBottomMat->setDiffuse(QColor(QRgb(0x888888)));
    Qt3DExtras::QPhongMaterial *armMat = new Qt3DExtras::QPhongMaterial();
    armMat->setDiffuse(QColor(QRgb(0x208040)));
    Qt3DExtras::QPhongMaterial *motorMat = new Qt3DExtras::QPhongMaterial();
    motorMat->setDiffuse(QColor(QRgb(0x111111)));
    Qt3DExtras::QPhongMaterial *propMat = new Qt3DExtras::QPhongMaterial();
    propMat->setDiffuse(QColor(QRgb(0x33aaff)));
    Qt3DExtras::QPhongMaterial *noseMat = new Qt3DExtras::QPhongMaterial();
    noseMat->setDiffuse(QColor(QRgb(0xff2222)));

    // 机身顶部
    Qt3DCore::QEntity *bodyTop = new Qt3DCore::QEntity(m_droneEntity);
    Qt3DExtras::QCuboidMesh *bodyTopMesh = new Qt3DExtras::QCuboidMesh();
    bodyTopMesh->setXExtent(0.8f);
    bodyTopMesh->setYExtent(0.8f);
    bodyTopMesh->setZExtent(0.1f);
    Qt3DCore::QTransform *bodyTopT = new Qt3DCore::QTransform();
    bodyTopT->setTranslation(QVector3D(0, 0, 0.1f));
    bodyTop->addComponent(bodyTopMesh);
    bodyTop->addComponent(bodyTopT);
    bodyTop->addComponent(bodyTopMat);

    // 机身底部
    Qt3DCore::QEntity *bodyBottom = new Qt3DCore::QEntity(m_droneEntity);
    Qt3DExtras::QCuboidMesh *bodyBottomMesh = new Qt3DExtras::QCuboidMesh();
    bodyBottomMesh->setXExtent(0.8f);
    bodyBottomMesh->setYExtent(0.8f);
    bodyBottomMesh->setZExtent(0.1f);
    Qt3DCore::QTransform *bodyBottomT = new Qt3DCore::QTransform();
    bodyBottomT->setTranslation(QVector3D(0, 0, -0.1f));
    bodyBottom->addComponent(bodyBottomMesh);
    bodyBottom->addComponent(bodyBottomT);
    bodyBottom->addComponent(bodyBottomMat);

    // 机头
    Qt3DCore::QEntity *nose = new Qt3DCore::QEntity(m_droneEntity);
    Qt3DExtras::QCuboidMesh *noseMesh = new Qt3DExtras::QCuboidMesh();
    noseMesh->setXExtent(0.5f);
    noseMesh->setYExtent(0.3f);
    noseMesh->setZExtent(0.1f);
    Qt3DCore::QTransform *noseT = new Qt3DCore::QTransform();
    noseT->setTranslation(QVector3D(0, 0.4f, 0.1f));
    nose->addComponent(noseMesh);
    nose->addComponent(noseT);
    nose->addComponent(noseMat);

    // 机臂参数
    const float armLen = 2.2f; // 机臂长度
    const float armR = 0.06f; // 机臂半径
    const float bodyHalf = 0.4f; // 机身半宽

    // 机臂1（右前）
    Qt3DCore::QEntity *arm1 = new Qt3DCore::QEntity(m_droneEntity);
    Qt3DExtras::QCylinderMesh *arm1Mesh = new Qt3DExtras::QCylinderMesh();// 形状：圆柱体
    arm1Mesh->setRadius(armR);
    arm1Mesh->setLength(armLen);
    arm1Mesh->setSlices(20);
    Qt3DCore::QTransform *arm1T = new Qt3DCore::QTransform();
    // 位置计算：√2/2（45度角的三角函数值）
    arm1T->setTranslation(QVector3D(bodyHalf + armLen/2*0.7071, bodyHalf + armLen/2*0.7071, 0));
    arm1T->setRotation(QQuaternion::fromEulerAngles(0, 0, -45));// 旋转-45度，呈X形
    arm1->addComponent(arm1Mesh);
    arm1->addComponent(arm1T);
    arm1->addComponent(armMat);

    // 机臂2（左前）
    Qt3DCore::QEntity *arm2 = new Qt3DCore::QEntity(m_droneEntity);
    Qt3DExtras::QCylinderMesh *arm2Mesh = new Qt3DExtras::QCylinderMesh();
    arm2Mesh->setRadius(armR);
    arm2Mesh->setLength(armLen);
    arm2Mesh->setSlices(20);
    Qt3DCore::QTransform *arm2T = new Qt3DCore::QTransform();
    arm2T->setTranslation(QVector3D(-bodyHalf - armLen/2*0.7071, bodyHalf + armLen/2*0.7071, 0));
    arm2T->setRotation(QQuaternion::fromEulerAngles(0, 0, 45));
    arm2->addComponent(arm2Mesh);
    arm2->addComponent(arm2T);
    arm2->addComponent(armMat);

    // 机臂3（右后）
    Qt3DCore::QEntity *arm3 = new Qt3DCore::QEntity(m_droneEntity);
    Qt3DExtras::QCylinderMesh *arm3Mesh = new Qt3DExtras::QCylinderMesh();
    arm3Mesh->setRadius(armR);
    arm3Mesh->setLength(armLen);
    arm3Mesh->setSlices(20);
    Qt3DCore::QTransform *arm3T = new Qt3DCore::QTransform();
    arm3T->setTranslation(QVector3D(bodyHalf + armLen/2*0.7071, -bodyHalf - armLen/2*0.7071, 0));
    arm3T->setRotation(QQuaternion::fromEulerAngles(0, 0, 45));
    arm3->addComponent(arm3Mesh);
    arm3->addComponent(arm3T);
    arm3->addComponent(armMat);

    // 机臂4（左后）
    Qt3DCore::QEntity *arm4 = new Qt3DCore::QEntity(m_droneEntity);
    Qt3DExtras::QCylinderMesh *arm4Mesh = new Qt3DExtras::QCylinderMesh();
    arm4Mesh->setRadius(armR);
    arm4Mesh->setLength(armLen);
    arm4Mesh->setSlices(20);
    Qt3DCore::QTransform *arm4T = new Qt3DCore::QTransform();
    arm4T->setTranslation(QVector3D(-bodyHalf - armLen/2*0.7071, -bodyHalf - armLen/2*0.7071, 0));
    arm4T->setRotation(QQuaternion::fromEulerAngles(0, 0, -45));
    arm4->addComponent(arm4Mesh);
    arm4->addComponent(arm4T);
    arm4->addComponent(armMat);

    // 电机参数
    const float motorR = 0.12f;
    const float motorH = 0.25f;
    const float motorPos = bodyHalf + armLen*0.7071;

    // 电机1
    Qt3DCore::QEntity *motor1 = new Qt3DCore::QEntity(m_droneEntity);
    Qt3DExtras::QCylinderMesh *motor1Mesh = new Qt3DExtras::QCylinderMesh();
    motor1Mesh->setRadius(motorR);
    motor1Mesh->setLength(motorH);
    motor1Mesh->setSlices(20);
    Qt3DCore::QTransform *motor1T = new Qt3DCore::QTransform();
    motor1T->setTranslation(QVector3D(motorPos, motorPos, motorH/2));
    motor1->addComponent(motor1Mesh);
    motor1->addComponent(motor1T);
    motor1->addComponent(motorMat);

    // 电机2
    Qt3DCore::QEntity *motor2 = new Qt3DCore::QEntity(m_droneEntity);
    Qt3DExtras::QCylinderMesh *motor2Mesh = new Qt3DExtras::QCylinderMesh();
    motor2Mesh->setRadius(motorR);
    motor2Mesh->setLength(motorH);
    motor2Mesh->setSlices(20);
    Qt3DCore::QTransform *motor2T = new Qt3DCore::QTransform();
    motor2T->setTranslation(QVector3D(-motorPos, motorPos, motorH/2));
    motor2->addComponent(motor2Mesh);
    motor2->addComponent(motor2T);
    motor2->addComponent(motorMat);

    // 电机3
    Qt3DCore::QEntity *motor3 = new Qt3DCore::QEntity(m_droneEntity);
    Qt3DExtras::QCylinderMesh *motor3Mesh = new Qt3DExtras::QCylinderMesh();
    motor3Mesh->setRadius(motorR);
    motor3Mesh->setLength(motorH);
    motor3Mesh->setSlices(20);
    Qt3DCore::QTransform *motor3T = new Qt3DCore::QTransform();
    motor3T->setTranslation(QVector3D(motorPos, -motorPos, motorH/2));
    motor3->addComponent(motor3Mesh);
    motor3->addComponent(motor3T);
    motor3->addComponent(motorMat);

    // 电机4
    Qt3DCore::QEntity *motor4 = new Qt3DCore::QEntity(m_droneEntity);
    Qt3DExtras::QCylinderMesh *motor4Mesh = new Qt3DExtras::QCylinderMesh();
    motor4Mesh->setRadius(motorR);
    motor4Mesh->setLength(motorH);
    motor4Mesh->setSlices(20);
    Qt3DCore::QTransform *motor4T = new Qt3DCore::QTransform();
    motor4T->setTranslation(QVector3D(-motorPos, -motorPos, motorH/2));
    motor4->addComponent(motor4Mesh);
    motor4->addComponent(motor4T);
    motor4->addComponent(motorMat);

    // 螺旋桨参数
    const float propBladeLen = 0.65f;
    const float propBladeW = 0.09f;
    const float propBladeH = 0.02f;
    const float propHeight = motorH + 0.05f;

    // 螺旋桨绘制函数
    auto drawProp = [&](float x, float y) {
        Qt3DCore::QEntity *b1 = new Qt3DCore::QEntity(m_droneEntity);
        Qt3DExtras::QCuboidMesh *m1 = new Qt3DExtras::QCuboidMesh();
        m1->setXExtent(propBladeLen);
        m1->setYExtent(propBladeW);
        m1->setZExtent(propBladeH);
        Qt3DCore::QTransform *t1 = new Qt3DCore::QTransform();
        t1->setTranslation(QVector3D(x + propBladeLen/2, y, propHeight));
        b1->addComponent(m1);
        b1->addComponent(t1);
        b1->addComponent(propMat);

        Qt3DCore::QEntity *b2 = new Qt3DCore::QEntity(m_droneEntity);
        Qt3DExtras::QCuboidMesh *m2 = new Qt3DExtras::QCuboidMesh();
        m2->setXExtent(propBladeLen);
        m2->setYExtent(propBladeW);
        m2->setZExtent(propBladeH);
        Qt3DCore::QTransform *t2 = new Qt3DCore::QTransform();
        t2->setTranslation(QVector3D(x - propBladeLen/2, y, propHeight));
        b2->addComponent(m2);
        b2->addComponent(t2);
        b2->addComponent(propMat);
    };

    // 绘制4个螺旋桨
    drawProp(motorPos, motorPos);
    drawProp(-motorPos, motorPos);
    drawProp(motorPos, -motorPos);
    drawProp(-motorPos, -motorPos);
}

void Drone3DView::setAttitude(float roll, float pitch, float yaw)
{
    m_droneTransform->setRotation(QQuaternion::fromEulerAngles(pitch, roll,yaw));
}
