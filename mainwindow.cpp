#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "infodialog.h"
#include "chip.h"

#include <QDebug>
#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QFileDialog>
#include <QMessageBox>
#include <QMouseEvent>
#include <QStringBuilder>

#ifndef DEBUG
#define DEBUG
#endif

//画出各种出入口
void paint_accessory(int size_x, int size_y, QPoint left_up_changed, QPoint right_down_changed, float grid_x, int x, int y, QPainter * painter, QString name)
{
    QFont font;
    font.setPointSize(8);
    font.setItalic(true);
    font.setBold(true);
    font.setFamily("Microsoft YaHei");
    painter->setFont(font);
    painter->setRenderHints(QPainter::TextAntialiasing);

    if (x == 1)
    {
        if (name == "废液出口")
            goto waste_go;
        QRectF rect(int(left_up_changed.x() - grid_x), int(left_up_changed.y() + (y - 1) * grid_x), int(grid_x), int(grid_x));
        painter->drawRect(rect);
        painter->drawText(rect, Qt::AlignCenter, name);
    }
    else if (x == size_x)
    {
        waste_go:
        QRectF rect(right_down_changed.x(), int(left_up_changed.y() + (y - 1) * grid_x), int(grid_x), int(grid_x));
        painter->drawRect(rect);
        painter->drawText(rect, Qt::AlignCenter, name);
    }
    else if (y == 1)
    {
        QRectF rect(int(left_up_changed.x() + (x - 1) * grid_x), int(left_up_changed.y() - grid_x), int(grid_x), int(grid_x));
        painter->drawRect(rect);
        painter->drawText(rect, Qt::AlignCenter, name);
    }
    else if (y == size_y)
    {
        QRectF rect(int(left_up_changed.x() + (x - 1) * grid_x), right_down_changed.y(), int(grid_x), int(grid_x));
        painter->drawRect(rect);
        painter->drawText(rect, Qt::AlignCenter, name);
    }
}

void MainWindow::timeChange()
{
    ui->lcdTime->display(chip->time_now);
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    //赋初值
    left_up.setX(100);
    left_up.setY(130);
    right_down.setX(900);
    right_down.setY(600);

    _output.SetValue(-100, -100);
    wash_input.SetValue(-100, -100);
    waste.SetValue(-100, -100);

    ui->setupUi(this);
    this->setFixedSize(this->size());

    //按钮先设置不可见、不可用
    ui->lastButton->setVisible(false);
    ui->nextButton->setVisible(false);
    ui->playButton->setVisible(false);
    ui->resetButton->setVisible(false);
    ui->lcdTime->setVisible(false);

    connect(ui->actioninput, SIGNAL(triggered()), this, SLOT(input()));
    connect(ui->actionExit, SIGNAL(triggered()), this, SLOT(close()));
    connect(ui->nextButton, SIGNAL(clicked()), this, SLOT(next()));
    connect(ui->lastButton, SIGNAL(clicked()), this, SLOT(last()));
    connect(ui->playButton, SIGNAL(clicked()), this, SLOT(play()));
    connect(ui->resetButton, SIGNAL(clicked()), this, SLOT(reset()));
}

MainWindow::~MainWindow()
{
    if (ui)
        delete ui;
    if (_input)
    {
        delete _input;
        _input = nullptr;
    }
    if (chip)
        delete chip;
}

void MainWindow::input()
{
    //读入“输入端口”的信息
    while (_input_num <= 0)
    {
        InfoDialog * infodialog = new InfoDialog(this);
        infodialog->setFixedSize(infodialog->size());
        infodialog->exec();
        delete infodialog;
    }
    start();
}

//获取chip信息
void MainWindow::get_data(const int &line, const int &row,
                          const int &inputnum, Position * inputptr,
                          const int &output_x, const int &output_y,
                          const bool &check_state)
{
    size_x = line;
    size_y = row;
    _clean = check_state;
    if (_clean)
    {
        wash_input.SetValue(1, 1);
        waste.SetValue(size_x, size_y);
    }

    _input_num = inputnum;
    _input = new Position[_input_num];
    for (int i = 0; i < _input_num; ++i)
        _input[i] = inputptr[i];
    _output.SetValue(output_x, output_y);
}

void MainWindow::paintEvent(QPaintEvent *event)
{
    if (_input_num == 0)
        return;

    reCalculate:
    left_up_changed = left_up;
    right_down_changed = right_down;

    //以下为了保证每格都为正方形，且图形最大化，用一个矩形范围内的最小每格长/宽代替另一个值
    grid_x = (right_down.x() - left_up.x()) / float(size_x);
    grid_y = (right_down.y() - left_up.y()) / float(size_y);
    if (grid_x < grid_y)
    {
        grid_y = grid_x;
        float diff = right_down.y() - left_up.y() - size_y * grid_x;
        left_up_changed.setY(int(left_up.y() + diff / 2));
        right_down_changed.setY(int(right_down.y() - diff / 2));
    }
    else if (grid_x > grid_y)
    {
        grid_x = grid_y;
        float diff = right_down.x() - left_up.x() - size_x * grid_x;
        left_up_changed.setX(int(left_up.x() + diff / 2));
        right_down_changed.setX(int(right_down.x() - diff / 2));
    }
    if (left_up_changed.x() - grid_x < 0)
    {
        left_up.setX(left_up.x() + grid_x / 2);
        goto reCalculate;
    }
    circle_size = grid_x / 8;

    QPen pen(Qt::black, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    QPainter painter;
    painter.begin(this);
    painter.setPen(pen);
    //画出边框
    painter.drawLine(left_up_changed, QPoint(right_down_changed.x(), left_up_changed.y()));
    painter.drawLine(left_up_changed, QPoint(left_up_changed.x(), right_down_changed.y()));
    painter.drawLine(right_down_changed, QPoint(right_down_changed.x(), left_up_changed.y()));
    painter.drawLine(right_down_changed, QPoint(left_up_changed.x(), right_down_changed.y()));

    //画出横竖线
    for (int i = 1; i < size_x; i++)
        painter.drawLine(int(left_up_changed.x() + i * grid_x), left_up_changed.y(), int(left_up_changed.x() + i * grid_x), right_down_changed.y());
    for (int i = 1; i < size_y; i++)
        painter.drawLine(left_up_changed.x(), int(left_up_changed.y() + i * grid_y), right_down_changed.x(), int(left_up_changed.y() + i * grid_y));

    //输入端口
    if (_input_num > 0)
    {
        for (int i = 0; i < _input_num; i++)
        {
            painter.setBrush(Qt::yellow);
            int x = _input[i]._x, y = _input[i]._y;
            paint_accessory(size_x, size_y, left_up_changed, right_down_changed, grid_x, x, y, &painter, tr("Input"));
        }
    }

    //输出端口
    painter.setBrush(Qt::green);
    int x = _output._x, y = _output._y;
    paint_accessory(size_x, size_y, left_up_changed, right_down_changed, grid_x, x, y, &painter, tr("Output"));

    if (_clean)
    {
        //清洗液进入口
        QColor blue = Qt::blue;
        blue = blue.lighter();
        painter.setBrush(blue);
        x = wash_input._x;
        y = wash_input._y;
        paint_accessory(size_x, size_y, left_up_changed, right_down_changed, grid_x, x, y, &painter, tr("Wash"));

        //废液出口
        painter.setBrush(Qt::red);
        x = waste._x;
        y = waste._y;
        paint_accessory(size_x, size_y, left_up_changed, right_down_changed, grid_x, x, y, &painter, tr("Waste"));
    }

    if (!judge_start)
        return;

    //画布的坐标转移
    painter_transformed.begin(this);
    QTransform transform;
    transform.translate(left_up_changed.x() - grid_x, left_up_changed.y() - grid_x);
    painter_transformed.setTransform(transform);
    transform.reset();

    //画污染情况\液滴
    paintPollutionDropBlock();

    //取消画笔
    painter_transformed.end();
}

void MainWindow::paintPollutionDropBlock()
{
    painter_transformed.setRenderHint(QPainter::Antialiasing);
    painter_transformed.setRenderHints(QPainter::TextAntialiasing);

    QColor green;
    green.setRgb(10, 140, 0);
    QColor green_background;
    green_background.setRgb(10, 140, 0, 30);

    QColor wash;
    wash.setRgb(20, 210, 230);

    QColor red;
    red.setRgb(255, 0, 0);

    QFont font;
    font.setPointSize(14);
    font.setFamily("Microsoft YaHei");

    QFont font_pollution;
    font_pollution.setPointSize(12);
    font_pollution.setItalic(true);
    font_pollution.setBold(true);
    font_pollution.setFamily("Microsoft YaHei");

    for (int i = 1; i <= size_x; i++)//i为横坐标
        for (int j = 1; j <= size_y; j++)//j为纵坐标
        {
            //污染的小圆圈，底色
            if (chip->pollution[i][j].getPollution() > 0)
            {
                painter_transformed.setPen(Qt::black);
                painter_transformed.setBrush(green_background);
                painter_transformed.drawRect(grid_x * i, grid_x * j, grid_x, grid_x);

                painter_transformed.setPen(green);
                painter_transformed.setBrush(green);
                painter_transformed.drawEllipse(chip->pollution[i][j].point[0], int(circle_size), int(circle_size));
                painter_transformed.drawEllipse(chip->pollution[i][j].point[1], int(circle_size), int(circle_size));
            }

            //液滴形状
            if (chip->drop[i][j].pollution_number > 0)
            {
                painter_transformed.setPen(Qt::NoPen);
                if (chip->drop[i][j].shape == Circle)
                {
                    painter_transformed.setBrush(Pollution_color[chip->drop[i][j].pollution_number]);
                    painter_transformed.drawEllipse(grid_x * i, grid_x * j, grid_x, grid_x);
                }
                else {
                    int bx = chip->drop[i][j].interaction_target._x, by = chip->drop[i][j].interaction_target._y;
                    QColor a_color = Pollution_color[chip->drop[i][j].pollution_number], b_color = Pollution_color[chip->drop[bx][by].pollution_number];
                    if (i == bx)
                    {
                        QLinearGradient g((i + 0.5) * grid_x, j * grid_x, (bx + 0.5) * grid_x, by * grid_x);
                        g.setColorAt(0, a_color);
                        g.setColorAt(1, b_color);
                        painter_transformed.setBrush(g);
                        painter_transformed.drawEllipse(i * grid_x, std::min(j, by) * grid_x, grid_x, 3 * grid_x);
                    }
                    else {
                        QLinearGradient g(i * grid_x, (j + 0.5) * grid_x, (bx + 1) * grid_x, (by + 0.5) * grid_x);
                        g.setColorAt(0, a_color);
                        g.setColorAt(1, b_color);
                        painter_transformed.setBrush(g);
                        painter_transformed.drawEllipse(std::min(i, bx) * grid_x, j * grid_x, 3 * grid_x, grid_x);
                    }
                }

                painter_transformed.setPen(Qt::black);
                painter_transformed.setFont(font_pollution);

                painter_transformed.drawText(grid_x * i, grid_x * j, grid_x, grid_x, Qt::AlignCenter, QString::number(chip->drop[i][j].pollution_number));
            }
            else if (chip->drop[i][j].pollution_number == 0)
            {
                painter_transformed.setPen(Qt::NoPen);
                painter_transformed.setBrush(wash);

                painter_transformed.drawEllipse(grid_x * i, grid_x * j, grid_x, grid_x);

                painter_transformed.setPen(Qt::black);
                painter_transformed.setFont(font_pollution);
                painter_transformed.drawText(grid_x * i, grid_x * j, grid_x, grid_x, Qt::AlignCenter, tr("wash"));
            }
            //被污染次数（格子上没有任何液滴的时候再写，否则写液滴污染序号）
            else if (chip->pollution[i][j].getPollution() > 0)
            {
                painter_transformed.setPen(Qt::black);
                painter_transformed.setFont(font);
                painter_transformed.drawText(grid_x * i, grid_x * j, grid_x, grid_x, Qt::AlignCenter, QString::number(chip->pollution[i][j].getPollution()));
            }


            //是否被block
            if (chip->blocked[i][j])
            {
                painter_transformed.setPen(Qt::black);
                QBrush brush(red, Qt::DiagCrossPattern);
                painter_transformed.setBrush(brush);
                painter_transformed.drawRect(grid_x * i, grid_x * j, grid_x, grid_x);
            }
        }
}

void MainWindow::start()
{
    //激活按钮
    ui->lastButton->setVisible(true);
    ui->nextButton->setVisible(true);
    ui->playButton->setVisible(true);
    ui->resetButton->setVisible(true);
    ui->lcdTime->setVisible(true);

    ui->lastButton->setEnabled(true);
    ui->nextButton->setEnabled(true);
    ui->playButton->setEnabled(true);
    ui->resetButton->setEnabled(true);
    ui->lcdTime->setEnabled(true);

    repaint();

    //将一些固定值给chip类
    if (chip == nullptr)
        chip = new Chip(this);
    else
    {
        chip->reset();
        chip->Visited.clearAll();
    }
    chip->size_x = size_x;
    chip->size_y = size_y;
    chip->_input = _input;
    chip->_input_num = _input_num;
    chip->_output = _output;
    chip->wash_input = wash_input;
    chip->waste = waste;
    chip->_clean = _clean;
    chip->grid_x = grid_x;
    chip->circle_size = circle_size;

    repaint();

    chip->calculate_pollution_point();

    //开始读入命令数据
    QString filename = "";
    while (filename == "")
        filename = QFileDialog::getOpenFileName(this, tr("选择数据读入文件"), "./", tr("文本文件(*.txt)"));
    chip->readOrder(filename);

    judge_start = true;
}

void MainWindow::killtimer(int & num)
{
    if (num > 0)
    {
        killTimer(num);
        num = -1000;
    }
}

void MainWindow::next()
{
    //正在清洗，则不管
    if (chip->wash_timer > 0)
    {
        killtimer(timerid);
        return;
    }

#ifdef DEBUG
    qDebug() << "#next# " << "before this action, time_now =" << chip->time_now;
#endif

    if (chip->time_now > chip->order[chip->order.size() - 1]._time && timerid >= 0){
        stopped = true;
        killtimer(timerid);
        QMessageBox::information(nullptr, QObject::tr("提示"), QObject::tr("自动播放已完成！"),
                                 QMessageBox::Yes, QMessageBox::Yes);
        return;
    }

    //出问题，不合规范的操作，停止自动播放
    if (!chip->next())
        killtimer(timerid);

    if (chip->time_now > chip->order[chip->order.size() - 1]._time)
        stopped = true;

    //清洗
    if (_clean)
        wash_pollution();

    timeChange();

    this->update();
}

void MainWindow::wash_pollution()
{
    if (stopped)
        return;
    chip->wash_timer = startTimer(150);
    chip->wash_pollution();
    repaint();
}

void MainWindow::last()
{
    stopped = false;
    if (chip->wash_timer > 0)
        return;

    killtimer(timerid);
    playing = false;

    chip->last();
    timeChange();
    this->update();

#ifdef DEBUG
    qDebug() << "#last# " << "after this action, time_now =" << chip->time_now;
#endif
}

void MainWindow::reset()
{
    stopped = false;
    ui->nextButton->setEnabled(true);
    ui->lastButton->setEnabled(true);
    ui->playButton->setEnabled(true);

    playing = false;
    killtimer(timerid);

    killtimer(chip->wash_timer);

#ifdef DEBUG
    qDebug() << "#reset# " << "after this action, time_now =" << chip->time_now;
#endif

    chip->reset();
    timeChange();
    this->update();
    QMessageBox::information(nullptr, QObject::tr("提示"), QObject::tr("已经复位到开始状态！"),
                             QMessageBox::Yes, QMessageBox::Yes);
}

void MainWindow::play()
{
    playing ^= true;
    if (chip->wash_timer > 0 && timerid >= 0)
    {
        killtimer(timerid);
        return;
    }
#ifdef DEBUG
    qDebug() << "#switch auto-play# " << "before this action, time_now =" << chip->time_now;
#endif

    if (timerid < 0)
        timerid = startTimer(1000);
    else
        killtimer(timerid);
}

void MainWindow::timerEvent(QTimerEvent * event)
{
    if (event->timerId() == timerid)
        emit next();

    if (event->timerId() == chip->wash_timer)
    {
        chip->wash_pollution();
        repaint();
    }
}

void MainWindow::mousePressEvent(QMouseEvent * event)
{
    //计算点击坐标
    int x = event->x() - left_up_changed.x() + grid_x, y = event->y() - left_up_changed.y() + grid_x;
    int x_geo = x / grid_x, y_geo = y / grid_x;

    //右键设置障碍
    if (event->button() == Qt::RightButton)
    {
        if (x_geo < 1 || x_geo > size_x || y_geo < 1 || y_geo > size_y)
            return;
        chip->blocked[x_geo][y_geo] ^= true;

        repaint();
    }

    //左键查看污染来源
    if (event->button() == Qt::LeftButton)
    {
        if (x_geo < 1 || x_geo > size_x || y_geo < 1 || y_geo > size_y)
            return;
        int pollution_num = chip->pollution[x_geo][y_geo].getPollution();
        QString info = QString("污染数量：%1").arg(QString::number(pollution_num));
        if (pollution_num != 0)
        {
            info = info % "\n污染来源：";
            int num = 0, test_num = 0;
            while (num < pollution_num)
            {
                if (chip->pollution[x_geo][y_geo].test(test_num))
                {
                    num++;
                    info = info % QString::number(test_num) % ';';
                }
                test_num++;
            }
        }

        QMessageBox::information(nullptr,
                                 QString("(%1,%2)污染情况").arg(QString::number(x_geo)).arg(QString::number(y_geo)),
                                 info, QMessageBox::Ok, QMessageBox::Ok);
    }
}

void MainWindow::stop()
{
    stopped = true;

    killtimer(timerid);
    killtimer(chip->wash_timer);
    ui->nextButton->setEnabled(false);
    ui->lastButton->setEnabled(false);
    ui->playButton->setEnabled(false);
}
