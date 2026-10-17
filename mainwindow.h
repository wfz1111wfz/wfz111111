#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

// 前向声明，杜绝循环包含
class SerialTab;
class FlightStatusTab;
class FlyCtrlSetTab;
class CalibrateTab; // 新增：飞行校准页

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    // 四个页面对象
    SerialTab *tab1;
    FlightStatusTab *tab2;
    FlyCtrlSetTab *tab3;
    CalibrateTab *tab4; // 新增：飞行校准页
};

#endif // MAINWINDOW_H
