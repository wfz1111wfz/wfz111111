#include "MainWindow.h"
#include "ui_MainWindow.h"
// 子页面头文件
#include "SerialTab.h"
#include "FlightStatusTab.h"
#include "FlyCtrlSetTab.h"
#include "CalibrateTab.h"
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // 1. 初始化串口调试页
    tab1 = new SerialTab(this);
    QVBoxLayout *layout1 = new QVBoxLayout(ui->tabSerial);
    layout1->addWidget(tab1);
    layout1->setContentsMargins(0, 0, 0, 0);

    // 2. 初始化飞控状态页
    tab2 = new FlightStatusTab(this, tab1);
    tab2->setSerialThread(tab1->getSerialThread());
    QVBoxLayout *layout2 = new QVBoxLayout(ui->tabStatus);
    layout2->addWidget(tab2);
    layout2->setContentsMargins(0, 0, 0, 0);

    //
    connect(tab1->getBluetoothThread(), &BluetoothThread::sig_dataSort, tab2, &FlightStatusTab::parseDataFrame);

    // 3. 初始化飞控设置页
    tab3 = new FlyCtrlSetTab(tab1->getSerialThread(), tab1->getBluetoothThread(), this);
    QVBoxLayout *layout3 = new QVBoxLayout(ui->tabSet);
    layout3->addWidget(tab3);
    layout3->setContentsMargins(0, 0, 0, 0);

    // 4. 初始化飞行校准页
    tab4 = new CalibrateTab(tab1->getSerialThread(), this);
    QVBoxLayout *layout4 = new QVBoxLayout(ui->tabCalibrate);
    layout4->addWidget(tab4);
    layout4->setContentsMargins(0, 0, 0, 0);

    // 默认显示串口调试页
    ui->tabWidget->setCurrentIndex(0);
}

MainWindow::~MainWindow()
{
    delete ui;
}