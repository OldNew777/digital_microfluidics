#ifndef CHIP_H
#define CHIP_H

#include <QPoint>
#include <QString>
#include <vector>

#include "position.h"
#include "pollution.h"
#include "order.h"
#include "Bitmap.h"
#include "liquid_drop.h"

class MainWindow;

class Chip
{
private:
    int max_times = 2000000;

    MainWindow * parent;

    bool judge_constraint(int lo, int hi);

    bool BFSway(Position now, Position target);
    void BFSgetArray(int &l, int &r, int * last, Position *position, bool (*gone)[14], Position now, Position target);
    void FindSpace(Position now);

    //命令执行
    void execute_order(Order & order_now);

    void action_INPUT(Order & order_now);
    void action_OUTPUT(Order & order_now);
    void action_MOVE(Order & order_now);
    void action_MERGE_1(Order & order_now);
    void action_MERGE_2(Order & order_now);
    void action_SPLIT_1(Order & order_now);
    void action_SPLIT_2(Order & order_now);

public:

    //settings
    int wash_timer = - 1000, lo_order = 0, hi_order = 0;
    Order wash_order[150];//前进命令

    Position wash_drop;//清洗液滴的当前位置
    int times = 0;//清洗液滴已经清洗了几个位置

    bool can_go[14][14], need_go[14][14], needWash = false;


    //----------------------------------
    int size_x = 1, size_y = 1;
    float grid_x, circle_size;
    bool _clean = false;

    int _input_num = 0;
    Position * _input = nullptr;
    Position _output, wash_input, waste;

	//----------------------------------
    //下面都是变化的数据
    int time_now = 0;

    //----------------------------------
    //以下数据为新增数据

    int pollution_max = 0;
    //新污染序号
    int tmp_pollution_number = 0;


    bool blocked[14][14];
    Pollution pollution[14][14];
    Bitmap Visited;

    LiquidDrop drop[14][14];
    //test用来判断动态静态约束，drop_next用来下一步走
    LiquidDrop test[14][14], drop_next[14][14];

    std::vector<Order> order;//reset的时候不要动
    std::vector<Order> order_reverse;


    //函数
    Chip(MainWindow * p = nullptr);

    void calculate_pollution_point();
    void readOrder(QString filename);

    bool next();
    void last();
    void reset();
    void wash_pollution();


    //测试函数
    void debug_pollution();
};

#endif // CHIP_H
