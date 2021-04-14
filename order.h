#ifndef ORDER_H
#define ORDER_H

#include "position.h"
#include <QString>
#include <QDebug>
#include <vector>

enum OrderType
{
    INPUT = 1,
    OUTPUT = 2,

	MERGE_1 = 3,
	MERGE_2 = 4,
	
	SPLIT_1 = 5,
	SPLIT_2 = 6,
	
    MOVE = 7,


    //这几个信号不会具体存在order里
    MERGE = 100,
    SPLIT = 101,
};


class Order
{
public:
    int _time = 0, pollution_number[3];
    bool forward = true;
    OrderType ordertype = MOVE;
    Position x[3];


    Order();
    Order(const Order& b);
    Order& operator=(const Order& b);
    void swap(Order& b);

    bool operator<(const Order& b)
    {
        return (_time < b._time);
    }
    bool operator<=(const Order& b)
    {
        return (_time <= b._time);
    }
    bool operator>(const Order& b)
    {
        return (_time > b._time);
    }
    bool operator>=(const Order& b)
    {
        return (_time >= b._time);
    }

    void setOrderType(QString type)
    {
        if (type == "Input")
            ordertype = INPUT;
        else if (type == "Output")
            ordertype = OUTPUT;
        else if (type == "Move")
            ordertype = MOVE;
        else if (type == "Merge")
            ordertype = MERGE;
        else if (type == "Split")
            ordertype = SPLIT;
    }
    void debugOutput()
    {
        qDebug() << ordertype << _time << forward;
        for (int i = 0; i < 3; i++)
            qDebug() << x[i]._x << x[i]._y << pollution_number[i];
        qDebug() << '\n';
    }
};

void quicksort(std::vector<Order> &s, int l, int r);

#endif // ORDER_H
