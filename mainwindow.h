#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPoint>
#include <QPainter>

#include "infodialog.h"
#include "position.h"
#include "pollutioncolor.h"

class Chip;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

    //系统参数
    int timerid = -1000;
    bool judge_start = false, playing = false, stopped = false;
    friend class Chip;

    //坐标定位
    QPoint left_up, right_down, left_up_changed, right_down_changed;
    QPainter painter_transformed;

    //chip信息
    int size_x = 1, size_y = 1, _input_num = 0;
    Position * _input = nullptr, _output, wash_input, waste;
    bool _clean = false;
    float grid_x = -100, grid_y = -100, circle_size;

    Chip * chip = nullptr;
    PollutionColor Pollution_color;


    //----------------------------------
    //以下为函数
    void start();
    void timeChange();

    void paintPollutionDropBlock();
    void stop();

    void paintEvent(QPaintEvent * event);
    void timerEvent(QTimerEvent * event);
    void mousePressEvent(QMouseEvent * event);

public slots:
    void killtimer(int & num);
    void input();
    void get_data(const int &row, const int &line,
                  const int &inputnum, Position * inputptr,
                  const int &output_x, const int &output_y,
                  const bool &check_state);
    void next();
    void last();
    void reset();
    void play();
    void wash_pollution();
};

#endif // MAINWINDOW_H
