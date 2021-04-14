#include "chip.h"
#include "mainwindow.h"

#include <QFile>
#include <vector>
#include <QStringList>
#include <QStringBuilder>
#include <QSound>

#include <QDebug>
#include <QMessageBox>

#ifndef DEBUG
#define DEBUG
#endif

bool Chip::next()
{
    if (time_now > order[order.size() - 1]._time)
    {
        QMessageBox::information(nullptr, QObject::tr("提示"), QObject::tr("已经做完最后一步！"),
                                 QMessageBox::Yes, QMessageBox::Yes);
        parent->stopped = true;
        return false;
    }

    //lo为命令队列里最低者的下标
    int lo = 0;
    while (lo != order.size())
    {
        if (order[lo]._time >= time_now)
            break;
        lo++;
    }
    if (lo == order.size() || order[lo]._time != time_now)
    {
        qDebug() << "no event on this second";
        time_now++;
        return true;
    }

    //hi为命令队列里最高者的下标
    int hi = lo;
    while (hi != order.size() && order[hi]._time == time_now)
        hi++;

    //[lo, hi)

    //判断合法与进行操作一起
    if (!judge_constraint(lo, hi))
    {
        QMessageBox::critical(nullptr, QObject::tr("警告"), QObject::tr("操作不满足约束条件！"),
                                 QMessageBox::Ok, QMessageBox::Ok);
        parent->stop();
        return false;
    }

    //加音效
    bool sound_to_play[8];
    for (int i = 0; i < 8; i++)
        sound_to_play[i] = false;
    for (int i = lo; i < hi; i++)
        sound_to_play[order[i].ordertype] = true;
    for (int i = 4; i < 8; i++)
        if (sound_to_play[i])
            QSound::play(QString(":/music/music/%1.wav").arg(QString::number(i)));

    time_now++;

#ifdef DEBUG
    debug_pollution();
#endif

    if (_clean)
        for (int i = 1; i <= size_x; i++)
            for (int j = 1; j <= size_y; j++)
                if (drop[i][j].pollution_number > 0 && pollution[i][j].getPollution() > 1)
                {
                    QMessageBox::critical(nullptr, QObject::tr("警告"), QObject::tr("液滴受到污染！"),
                                             QMessageBox::Ok, QMessageBox::Ok);
                    return true;
                }

    return true;
}

void Chip::last()
{
    if (time_now <= 0)
    {
        QMessageBox::information(nullptr, QObject::tr("提示"), QObject::tr("已经复位到开始状态！"),
                                 QMessageBox::Yes, QMessageBox::Yes);
        return;
    }

    //lo为命令队列里最低者的迭代器
    int lo = 0;
    while (lo != order_reverse.size())
    {
        if (order_reverse[lo]._time >= time_now)
            break;
        lo++;
    }
    if (lo == order_reverse.size() || order_reverse[lo]._time != time_now)
    {
        qDebug() << "no event on this second";
        time_now--;
        return;
    }

    //hi为命令队列里最高者的迭代器
    int hi = lo;
    while (hi != order_reverse.size() && order_reverse[hi]._time == time_now)
        hi++;

    //[lo, hi)


    //初始化
    for (int i = 0; i < 14; i ++)
        for (int j = 0; j < 14; j ++)
        {
            test[i][j] = drop[i][j];
            drop_next[i][j] = drop[i][j];
        }

    for (int i = lo; i < hi; i++)
    {
        //test不需要改变液滴形状，只用来判断约束条件
        execute_order(order_reverse[i]);
    }

    //正式赋值
    for (int i = 0; i < 14; i++)
        for (int j = 0; j < 14; j++)
            drop[i][j] = drop_next[i][j];

    time_now--;

#ifdef DEBUG
    debug_pollution();
#endif
}

void Chip::execute_order(Order & order_now)
{
    switch (order_now.ordertype)
    {
    case INPUT:
        action_INPUT(order_now);
        break;

    case OUTPUT:
        action_OUTPUT(order_now);
        break;

    case MOVE:
        action_MOVE(order_now);
        break;

    case MERGE_1:
        action_MERGE_1(order_now);
        break;

    case MERGE_2:
        action_MERGE_2(order_now);
        break;

    case SPLIT_1:
        action_SPLIT_1(order_now);
        break;

    case SPLIT_2:
        action_SPLIT_2(order_now);
        break;
    }
}

void Chip::calculate_pollution_point()
{
    qDebug() << grid_x;

    for (int i = 1; i <= size_x; i++)//i为横坐标
        for (int j = 1; j <= size_y; j++)//j为纵坐标
        {
            float x = i * grid_x + circle_size + (rand() % int(grid_x - 2 * circle_size));
            float y = j * grid_x + circle_size + (rand() % int(grid_x - 2 * circle_size));
            pollution[i][j].point[0].setX(x);
            pollution[i][j].point[0].setY(y);
            x = i * grid_x + circle_size + (rand() % int(grid_x - 2 * circle_size));
            y = j * grid_x + circle_size + (rand() % int(grid_x - 2 * circle_size));
            pollution[i][j].point[1].setX(x);
            pollution[i][j].point[1].setY(y);
        }
}

void Chip::reset()
{
    parent->killtimer(wash_timer);
    lo_order = 0;
    hi_order = 0;
    times = 0;
    time_now = 0;
    pollution_max = 0;
    wash_drop.SetValue(-100, -100);
    for (int i = 0; i < 14; i ++)
        for (int j = 0; j < 14; j ++)
        {
            blocked[i][j] = false;
            pollution[i][j].reset();
            drop[i][j].clear();
        }
    Visited.clearAll();
}

Chip::Chip(MainWindow * p)
{
    parent = p;
    for (int i = 0; i < 14; i ++)
        for (int j = 0; j < 14; j ++)
            blocked[i][j] = false;
    wash_drop.SetValue(-100, -100);
}

void Chip::readOrder(QString filename)
{
    order.clear();
    int now = -1;

    QFile file(filename);
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QString tmp = "";
    tmp = file.readLine();
    while (tmp != "")
    {
        //读入一列数据，并做分割处理
        QStringList list = tmp.split(',');
        QString first2 = list[0].left(2);

        if (first2 == "In" || first2 == "Me" || first2 == "Sp")
        {
            list.insert(1, list[0].mid(6));
            list[0] = list[0].left(5);
        }
        else if (first2 == "Ou")
        {
            list.insert(1, list[0].mid(7));
            list[0] = list[0].left(6);
        }
        else if (first2 == "Mo")
        {
            list.insert(1, list[0].mid(5));
            list[0] = list[0].left(4);
        }
        else if (first2 == "Mi")
        {
            list.insert(1, list[0].mid(4));
            list[0] = list[0].left(3);
        }

        list.replaceInStrings(";", "");
        list.replaceInStrings("\n", "");

#ifdef DEBUG
        qDebug() << list;
#endif

        //将命令分割成每秒，再存储命令
        //Mix命令的参数个数不限，故单独处理
        if (list[0] == "Mix"){
            int time = list[1].toInt() - 1;
            for (int i = 2; i <= list.length() - 4; i += 2)
            {
                time++;
                now++;
                order.push_back(Order());
                order[now].setOrderType("Move");
                order[now]._time = time;
                order[now].x[0].SetValue(list[i].toInt(), list[i + 1].toInt());
                order[now].x[1].SetValue(list[i + 2].toInt(), list[i + 3].toInt());
            }
            tmp = "";
            tmp = file.readLine();
            continue;
        }

        //其余
        order.push_back(Order());
        now++;
        order[now].setOrderType(list[0]);
        order[now]._time = list[1].toInt();

        order[now].x[0].SetValue(list[2].toInt(), list[3].toInt());
        if (order[now].ordertype > 2)
            order[now].x[1].SetValue(list[4].toInt(), list[5].toInt());

        //分离，分两阶段
        if (order[now].ordertype == SPLIT)
        {
            order[now].x[2].SetValue(list[6].toInt(), list[7].toInt());
            order[now].ordertype = SPLIT_1;
            now++;
            order.push_back(Order(order[now - 1]));
            order[now]._time++;
            order[now].ordertype = SPLIT_2;
        }

        //合并，分两阶段
        else if (order[now].ordertype == MERGE)
        {
            order[now].x[2].SetValue(order[now].x[0]._x, order[now].x[0]._y);
            order[now].x[0].SetValue((order[now].x[1]._x + order[now].x[2]._x) / 2, (order[now].x[1]._y + order[now].x[2]._y) / 2);
            order[now].ordertype = MERGE_1;
            now++;
            order.push_back(Order(order[now - 1]));
            order[now]._time++;
            order[now].ordertype = MERGE_2;
        }

        tmp = "";
        tmp = file.readLine();
    }

    //以时间排序
//    std::sort(order.begin(), order.end());
    quicksort(order, 0, order.size() - 1);

    file.close();
}

bool Chip::judge_constraint(int lo, int hi)
{
    //初始化
    for (int i = 0; i < 14; i ++)
        for (int j = 0; j < 14; j ++)
        {
            test[i][j] = drop[i][j];
            drop_next[i][j] = drop[i][j];
        }

    for (int i = lo; i < hi; i++)
    {
        int num = order_reverse.size();
        if (!Visited.test(i))
        {
            order_reverse.push_back(Order());
            order_reverse[num].forward = false;
            order_reverse[num]._time = time_now + 1;
            for (int q = 0; q < 3; q++)
                order_reverse[num].x[q] = order[i].x[q];
        }

        //test不需要改变液滴形状，只用来判断约束条件
        switch (order[i].ordertype)
        {
        case INPUT:
        {
            action_INPUT(order[i]);

            //记录order_reverse
            if (!Visited.test(i))
                order_reverse[num].ordertype = OUTPUT;
            break;
        }
        case OUTPUT:
        {
            action_OUTPUT(order[i]);

            //记录order_reverse
            if (!Visited.test(i))
                order_reverse[num].ordertype = INPUT;
            break;
        }
        case MOVE:
        {
            action_MOVE(order[i]);

            //记录order_reverse
            if (!Visited.test(i))
            {
                order_reverse[num].ordertype = MOVE;
                order_reverse[num].x[0].swap(order_reverse[num].x[1]);
            }
            break;
        }
        case MERGE_1:
        {
            action_MERGE_1(order[i]);

            //记录order_reverse
            if (!Visited.test(i))
                order_reverse[num].ordertype = SPLIT_2;
            break;
        }
        case MERGE_2:
        {
            action_MERGE_2(order[i]);

            //记录order_reverse
            if (!Visited.test(i))
                order_reverse[num].ordertype = SPLIT_1;
            break;
        }
        case SPLIT_1:
        {
            action_SPLIT_1(order[i]);

            //记录order_reverse
            if (!Visited.test(i))
                order_reverse[num].ordertype = MERGE_2;
            break;
        }
        case SPLIT_2:
        {
            action_SPLIT_2(order[i]);

            //记录order_reverse
            if (!Visited.test(i))
                order_reverse[num].ordertype = MERGE_1;
            break;
        }
        }

        if (!Visited.test(i))
            for (int q = 0; q < 3; q++)
                order_reverse[num].pollution_number[q] = order[i].pollution_number[q];

        Visited.set(i);
    }

    //正式赋值
    for (int i = 0; i < 14; i++)
        for (int j = 0; j < 14; j++)
            drop[i][j] = drop_next[i][j];

    //检测
    for (int i = 1; i <= size_x; i++)
        for (int j = 1; j <= size_y; j++)
        {
            for (int p = i - 1; p <= i + 1; p++)
                for (int q = j - 1; q <= j + 1; q++)
                {
                    if (p == i && q == j)
                        continue;
                    if (illegal(test[i][j], test[p][q], i, j, p, q))
                        return false;
                }
        }

    return true;
}

void Chip::action_INPUT(Order & order_now)
{
    //判断是否合法
    bool beside = false;
    if (order_now.pollution_number[0] == 0)
    {
        if (order_now.x[0] == wash_input)
        {
            beside = true;
            wash_drop = wash_input;
        }
    }
    else if (order_now.forward)
    {
        for (int i = 0; i < _input_num; i++)
            if (_input[i] == order_now.x[0])
            {
                beside = true;
                break;
            }
    }
    else if (_output == order_now.x[0])
        beside = true;
    if (!beside)
    {
        QMessageBox::critical(nullptr, QObject::tr("警告"), QObject::tr("输入/出液滴不在输入/出端口附近！"),
                              QMessageBox::Ok, QMessageBox::Ok);
        parent->stop();
        return;
    }

    //更新液滴污染号码
    LiquidDrop tmp_input;
    if (order_now.pollution_number[0] < 0)
    {
        tmp_input.pollution_number = ++tmp_pollution_number;
        order_now.pollution_number[0] = tmp_pollution_number;
    }
    else
        tmp_input.pollution_number = order_now.pollution_number[0];

    if (order_now.forward)
    {
        if (pollution[order_now.x[0]._x][order_now.x[0]._y].getPollution() > 0 && order_now.pollution_number[0] == 0)
            times++;
        pollution[order_now.x[0]._x][order_now.x[0]._y].pollute(tmp_input.pollution_number);
    }

    test[order_now.x[0]._x][order_now.x[0]._y] = tmp_input;
    drop_next[order_now.x[0]._x][order_now.x[0]._y] = tmp_input;
}

void Chip::action_OUTPUT(Order & order_now)
{
    if (order_now.pollution_number[0] < 0)
        order_now.pollution_number[0] = drop[order_now.x[0]._x][order_now.x[0]._y].pollution_number;

    bool beside = false;
    if (order_now.forward)
    {
        if (order_now.pollution_number[0] > 0 && _output == order_now.x[0])
            beside = true;
        else if (waste == order_now.x[0])
        {
            beside = true;
            wash_drop.SetValue(-100, -100);
            times = 0;
        }
    }
    else {
        for (int i = 0; i < _input_num; i++)
            if (_input[i] == order_now.x[0])
            {
                beside = true;
                break;
            }
    }
    if (!beside)
    {
        QMessageBox::critical(nullptr, QObject::tr("警告"), QObject::tr("输入/出液滴不在输入/出端口附近！"),
                              QMessageBox::Ok, QMessageBox::Ok);
        parent->stop();
        return;
    }

    order_now.pollution_number[0] = drop[order_now.x[0]._x][order_now.x[0]._y].pollution_number;
    drop_next[order_now.x[0]._x][order_now.x[0]._y].clear();
    if (!order_now.forward)
        pollution[order_now.x[0]._x][order_now.x[0]._y].clean(order_now.pollution_number[0]);
}

void Chip::action_MOVE(Order & order_now)
{
    if (order_now.pollution_number[0] < 0)
    {
        if (drop[order_now.x[0]._x][order_now.x[0]._y].pollution_number < 0)
        {
            QMessageBox::critical(nullptr, QObject::tr("警告"), QObject::tr("Move操作的位置没有液滴！"),
                                  QMessageBox::Ok, QMessageBox::Ok);
            parent->stop();
            return;
        }
        order_now.pollution_number[0] = drop[order_now.x[0]._x][order_now.x[0]._y].pollution_number;
    }
    if (order_now.pollution_number[0] == 0)
        wash_drop = order_now.x[1];

    test[order_now.x[1]._x][order_now.x[1]._y] = drop[order_now.x[0]._x][order_now.x[0]._y];

    if (order_now.forward)
    {
        if (order_now.pollution_number[0] > 0)
            pollution[order_now.x[1]._x][order_now.x[1]._y].pollute(order_now.pollution_number[0]);
        else if (pollution[order_now.x[1]._x][order_now.x[1]._y].getPollution() > 0 && times < max_times)
        {
            times++;
            pollution[order_now.x[1]._x][order_now.x[1]._y].pollute(0);
        }
    }
    else
        pollution[order_now.x[0]._x][order_now.x[0]._y].clean(drop[order_now.x[0]._x][order_now.x[0]._y].pollution_number);

    drop_next[order_now.x[1]._x][order_now.x[1]._y] = drop[order_now.x[0]._x][order_now.x[0]._y];
    drop_next[order_now.x[0]._x][order_now.x[0]._y].clear();

    if (order_now.pollution_number[0] == 0 && times == max_times)
        hi_order = lo_order;
}

void Chip::action_MERGE_1(Order & order_now)
{
    if (order_now.pollution_number[1] < 0 || order_now.pollution_number[2] < 0)
    {
        if (drop[order_now.x[1]._x][order_now.x[1]._y].pollution_number < 0 || drop[order_now.x[2]._x][order_now.x[2]._y].pollution_number < 0)
        {
            QMessageBox::critical(nullptr, QObject::tr("警告"), QObject::tr("Merge操作的位置没有液滴！"),
                                  QMessageBox::Ok, QMessageBox::Ok);
            parent->stop();
            return;
        }
        order_now.pollution_number[1] = drop[order_now.x[1]._x][order_now.x[1]._y].pollution_number;
        order_now.pollution_number[2] = drop[order_now.x[2]._x][order_now.x[2]._y].pollution_number;
    }
    drop_next[order_now.x[1]._x][order_now.x[1]._y].shape = Ellipse;
    drop_next[order_now.x[2]._x][order_now.x[2]._y].shape = Ellipse;
    drop_next[order_now.x[1]._x][order_now.x[1]._y].interaction_target.SetValue(order_now.x[2]._x, order_now.x[2]._y);
    drop_next[order_now.x[2]._x][order_now.x[2]._y].interaction_target.SetValue(order_now.x[1]._x, order_now.x[1]._y);
}

void Chip::action_MERGE_2(Order & order_now)
{
    if (order_now.pollution_number[1] < 0 || order_now.pollution_number[2] < 0)
    {
        order_now.pollution_number[1] = drop[order_now.x[1]._x][order_now.x[1]._y].pollution_number;
        order_now.pollution_number[2] = drop[order_now.x[2]._x][order_now.x[2]._y].pollution_number;
    }
    LiquidDrop tmp_drop;
    if (order_now.pollution_number[0] < 0)
    {
        tmp_drop.pollution_number = ++tmp_pollution_number;
        order_now.pollution_number[0] = tmp_pollution_number;
    }
    else
        tmp_drop.pollution_number = order_now.pollution_number[0];
    if (order_now.forward)
        pollution[order_now.x[0]._x][order_now.x[0]._y].pollute(tmp_drop.pollution_number);
    else
    {
        pollution[order_now.x[1]._x][order_now.x[1]._y].clean(order_now.pollution_number[1]);
        pollution[order_now.x[2]._x][order_now.x[2]._y].clean(order_now.pollution_number[2]);
    }

    drop_next[order_now.x[0]._x][order_now.x[0]._y] = tmp_drop;
    drop_next[order_now.x[1]._x][order_now.x[1]._y].clear();
    drop_next[order_now.x[2]._x][order_now.x[2]._y].clear();
}

void Chip::action_SPLIT_1(Order & order_now)
{
    if (order_now.pollution_number[0] < 0)
    {
        if (drop[order_now.x[0]._x][order_now.x[0]._y].pollution_number < 0)
        {
            QMessageBox::critical(nullptr, QObject::tr("警告"), QObject::tr("Split操作的位置没有液滴！"),
                                  QMessageBox::Ok, QMessageBox::Ok);
            parent->stop();
            return;
        }
        order_now.pollution_number[0] = drop[order_now.x[0]._x][order_now.x[0]._y].pollution_number;
    }
    LiquidDrop tmp1, tmp2;
    if (order_now.pollution_number[1] < 0 || order_now.pollution_number[2] < 0)
    {
        tmp1.pollution_number = ++tmp_pollution_number;
        order_now.pollution_number[1] = tmp_pollution_number;
        tmp2.pollution_number = ++tmp_pollution_number;
        order_now.pollution_number[2] = tmp_pollution_number;
    }
    else{
        tmp1.pollution_number = order_now.pollution_number[1];
        tmp2.pollution_number = order_now.pollution_number[2];
    }
    tmp1.shape = Ellipse;
    tmp2.shape = Ellipse;
    tmp1.interaction_target.SetValue(order_now.x[2]._x, order_now.x[2]._y);
    tmp2.interaction_target.SetValue(order_now.x[1]._x, order_now.x[1]._y);

    if (order_now.forward)
    {
        pollution[order_now.x[1]._x][order_now.x[1]._y].pollute(tmp1.pollution_number);
        pollution[order_now.x[2]._x][order_now.x[2]._y].pollute(tmp2.pollution_number);
    }
    else {
        pollution[order_now.x[0]._x][order_now.x[0]._y].clean(order_now.pollution_number[0]);
    }

    test[order_now.x[1]._x][order_now.x[1]._y] = tmp1;
    test[order_now.x[2]._x][order_now.x[2]._y] = tmp2;
    test[order_now.x[0]._x][order_now.x[0]._y].clear();
    drop_next[order_now.x[1]._x][order_now.x[1]._y] = tmp1;
    drop_next[order_now.x[2]._x][order_now.x[2]._y] = tmp2;
    drop_next[order_now.x[0]._x][order_now.x[0]._y].clear();
}

void Chip::action_SPLIT_2(Order & order_now)
{
    if (order_now.pollution_number[1] < 0 || order_now.pollution_number[2] < 0)
    {
        order_now.pollution_number[1] = drop[order_now.x[1]._x][order_now.x[1]._y].pollution_number;
        order_now.pollution_number[2] = drop[order_now.x[2]._x][order_now.x[2]._y].pollution_number;
    }
    drop_next[order_now.x[1]._x][order_now.x[1]._y].shape = Circle;
    drop_next[order_now.x[2]._x][order_now.x[2]._y].shape = Circle;
}

void Chip::debug_pollution()
{
    for (int i = 1; i <= size_y; i++)
    {
        QString str = "";
        for (int j = 1; j <= size_x; j++)
            str = str % QString::number(drop[j][i].pollution_number) % ' ';
        qDebug() << str;
    }
}

void Chip::wash_pollution()
{
    if (parent->stopped)
        return;

    if (lo_order >= hi_order)//判断是否还需要清洗
    {
        //可能是刚开始清洗，也可能是洗完一个阶段。但都要更新can_go和need_go
        needWash = false;
        for (int i = 0; i < 14; i++)
            for (int j = 0; j < 14; j++)
            {
                if (blocked[i][j])
                    can_go[i][j] = false;
                else
                    can_go[i][j] = true;
                need_go[i][j] = false;
            }
        for (int i = 0; i < 14; i++)
            for (int j = 0; j < 14; j++)
                if (drop[i][j].pollution_number > 0)
                {
                    for (int p = i - 1; p <= i + 1; p++)
                        for (int q = j - 1; q <= j + 1; q++)
                            can_go[p][q] = false;
                }
        for (int i = 0; i < 14; i++)
            for (int j = 0; j < 14; j++)
                if (can_go[i][j] && pollution[i][j].getPollution() > 0)
                {
                    need_go[i][j] = true;
                    needWash = true;
                }

        if (!needWash)
        {
            if (wash_drop._x > 0 && wash_drop._y > 0)//洗完最后一个污染点，要去废液出口waste
            {
                if (!BFSway(wash_drop, waste))
                    FindSpace(wash_drop);
            }
            else//这一轮不要动
            {
                parent->killtimer(wash_timer);
                if (parent->playing)
                    parent->timerid = parent->startTimer(1000);
                return;
            }
        }
        else if (needWash)
        {
            if (times < max_times)
            {
                bool exit = false;
                //选一个污染点作为目标开始向它前进，清洗
                for (int i = 1; i <= size_x; i++)
                {
                    if (exit)
                        break;
                    for (int j = 1; j <= size_y; j++)
                    {
                        if (exit)
                            break;
                        if (need_go[i][j])
                        {
                            if (BFSway(wash_drop, Position(i, j)));
                                exit = true;
                        }
                    }
                }
                if (wash_drop._x > 0 && wash_drop._y > 0 && !exit)//芯片上已经有了washdrop，但是到不了清洗点
                    if (!BFSway(wash_drop, waste))
                        FindSpace(wash_drop);
            }
            else {
                if (!BFSway(wash_drop, waste))
                    FindSpace(wash_drop);
            }
        }
    }

    if (lo_order == hi_order)
    {
        parent->killtimer(wash_timer);
        if (parent->playing)
            parent->timerid = parent->startTimer(1000);
        return;
    }

    //-----------------------------------------
    //执行现有的order

    //初始化
    for (int i = 0; i < 14; i ++)
        for (int j = 0; j < 14; j ++)
        {
            test[i][j] = drop[i][j];
            drop_next[i][j] = drop[i][j];
        }

    execute_order(wash_order[lo_order]);
    lo_order++;

    //正式赋值
    for (int i = 0; i < 14; i++)
        for (int j = 0; j < 14; j++)
            drop[i][j] = drop_next[i][j];
}

bool Chip::BFSway(Position now, Position target)
{
    lo_order = 0;
    hi_order = 0;
    if (now._x < 0 || now._y < 0)//添加清洗液滴
    {
        if (!can_go[wash_input._x][wash_input._y])
            return true;
        hi_order++;
        wash_order[0].x[0] = wash_input;
        now = wash_input;
        wash_order[0].ordertype = INPUT;
        wash_order[0].pollution_number[0] = 0;
    }
    if (now == target && target == waste)//已经在waste旁边了
    {
        hi_order++;
        wash_order[0].x[0] = waste;
        wash_order[0].ordertype = OUTPUT;
        wash_order[0].pollution_number[0] = 0;
        return true;
    }

    //BFS寻路
    int l = 0, r = 0, last[150];
    Position position[150];
    bool gone[14][14];

    BFSgetArray(l, r, last, position, gone, now, target);

    //如果找不到路怎么办？
    if (position[r] != target)
    {
        lo_order = 0;
        hi_order = 0;
        return false;
    }

    //路径转换成清洗液滴的命令
    int num = 0;
    Position way[150];
    while (last[r] != r)
    {
        way[num++] = position[r];
        r = last[r];
    }
    way[num--] = now;
    for (int i = num; i >= 0; i--)
    {
        wash_order[hi_order].pollution_number[0] = 0;
        wash_order[hi_order].x[0] = way[i + 1];
        wash_order[hi_order].x[1] = way[i];
        wash_order[hi_order].ordertype = MOVE;
        hi_order++;
    }
    return true;
}

void Chip::FindSpace(Position now)
{
    bool space[14][14], gone[14][14];
    for (int i = 0; i < 14; i++)
        for (int j = 0; j < 14; j++)
        {
            space[i][j] = can_go[i][j];
            gone[i][j] = false;
        }

    //有液滴的周围5*5格子都设置为不可在区域
    for (int i = 1; i <= size_x; i++)
        for (int j = 1; j <= size_y; j++)
            if (drop[i][j].pollution_number > 0)
            {
                for (int p = i - 2; p <= i + 2; p++)
                    for (int q = j - 2; q <= j + 2; q++)
                        if (p >= 0 && p < 14 && q >= 0 && q < 14)
                            space[p][q] = false;
            }

    //input液滴入口设置为不可在区域
    for (int i = 0; i < _input_num; i++)
        for (int p = _input[i]._x - 1; p <= _input[i]._x + 1; p++)
            for (int q = _input[i]._y - 1; q <= _input[i]._y + 1; q++)
                space[p][q] = false;
    //output液滴出口设置为不可在区域
    for (int p = _output._x - 1; p <= _output._x + 1; p++)
        for (int q = _output._y - 1; q <= _output._y + 1; q++)
            space[p][q] = false;

    //如果所在格子可以暂时待着，就不动
    if (space[now._x][now._y])
    {
        lo_order = 0;
        hi_order = 0;
        return;
    }

    int l, r, last[150];
    Position position[150], tmp_target;

    //找最近的空地，宽搜过去
    int left = 0, right = 0;
    position[0] = now;
    gone[now._x][now._y] = true;

    while (left <= right)
    {
        if (position[left]._x < size_x && !gone[position[left]._x + 1][position[left]._y] && can_go[position[left]._x + 1][position[left]._y])
        {
            right++;
            position[right].SetValue(position[left]._x + 1, position[left]._y);
            gone[position[right]._x][position[right]._y] = true;
            if (space[position[right]._x][position[right]._y])
                break;
        }
        if (position[left]._y < size_y && !gone[position[left]._x][position[left]._y + 1] && can_go[position[left]._x][position[left]._y + 1])
        {
            right++;
            position[right].SetValue(position[left]._x, position[left]._y + 1);
            gone[position[right]._x][position[right]._y] = true;
            if (space[position[right]._x][position[right]._y])
                break;
        }
        if (position[left]._x > 1 && !gone[position[left]._x - 1][position[left]._y] && can_go[position[left]._x - 1][position[left]._y])
        {
            right++;
            position[right].SetValue(position[left]._x - 1, position[left]._y);
            gone[position[right]._x][position[right]._y] = true;
            if (space[position[right]._x][position[right]._y])
                break;
        }
        if (position[left]._y > 1 && !gone[position[left]._x][position[left]._y - 1] && can_go[position[left]._x][position[left]._y - 1])
        {
            right++;
            position[right].SetValue(position[left]._x, position[left]._y - 1);
            gone[position[right]._x][position[right]._y] = true;
            if (space[position[right]._x][position[right]._y])
                break;
        }
        left++;
    }

    //找不到空地
    if (!space[position[right]._x][position[right]._y])
    {
        lo_order = 0;
        hi_order = 0;
        return;
    }
    else {
        //position[right]是最近的可以暂时待着的空地
        BFSgetArray(l, r, last, position, gone, now, position[right]);

        //路径转换成清洗液滴的命令
        int num = 0;
        Position way[150];
        while (last[r] != r)
        {
            way[num++] = position[r];
            r = last[r];
        }
        way[num--] = now;
        for (int i = num; i >= 0; i--)
        {
            wash_order[hi_order].pollution_number[0] = 0;
            wash_order[hi_order].x[0] = way[i + 1];
            wash_order[hi_order].x[1] = way[i];
            wash_order[hi_order].ordertype = MOVE;
            hi_order++;
        }
    }
}

void Chip::BFSgetArray(int &l, int &r, int * last, Position *position, bool (*gone)[14], Position now, Position target)
{
    l = 0;
    r = 0;
    last[0] = 0;
    position[0] = now;
    if (now == target)
        return;

    //gone每次清空
    for (int i = 0; i < 14; i++)
        for (int j = 0; j < 14; j++)
            gone[i][j] = false;
    gone[now._x][now._y] = true;

    while (l <= r)
    {
        if (position[l]._x < size_x && !gone[position[l]._x + 1][position[l]._y] && can_go[position[l]._x + 1][position[l]._y])
        {
            r++;
            last[r] = l;
            position[r].SetValue(position[l]._x + 1, position[l]._y);
            gone[position[r]._x][position[r]._y] = true;
            if (position[r] == target)
                break;
        }
        if (position[l]._y < size_y && !gone[position[l]._x][position[l]._y + 1] && can_go[position[l]._x][position[l]._y + 1])
        {
            r++;
            last[r] = l;
            position[r].SetValue(position[l]._x, position[l]._y + 1);
            gone[position[r]._x][position[r]._y] = true;
            if (position[r] == target)
                break;
        }
        if (position[l]._x > 1 && !gone[position[l]._x - 1][position[l]._y] && can_go[position[l]._x - 1][position[l]._y])
        {
            r++;
            last[r] = l;
            position[r].SetValue(position[l]._x - 1, position[l]._y);
            gone[position[r]._x][position[r]._y] = true;
            if (position[r] == target)
                break;
        }
        if (position[l]._y > 1 && !gone[position[l]._x][position[l]._y - 1] && can_go[position[l]._x][position[l]._y - 1])
        {
            r++;
            last[r] = l;
            position[r].SetValue(position[l]._x, position[l]._y - 1);
            gone[position[r]._x][position[r]._y] = true;
            if (position[r] == target)
                break;
        }
        l++;
    }
}
