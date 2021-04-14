#include "order.h"

#ifndef DEBUG
#define DEBUG
#include <QString>
#include <QStringBuilder>
#endif

void quicksort(std::vector<Order> &s, int l, int r)
{
    int i, j;
    Order x;
    if (l < r)
    {
        i = l;
        j = r;
        x = s[i];
        while (i < j)
        {
            while(i < j && s[j] > x)
                j--;
            if(i < j)
                s[i++] = s[j];

            while(i < j && s[i] < x)
                i++;
            if(i < j)
                s[j--] = s[i];
        }
        s[i] = x;

        quicksort(s, l, i-1);
        quicksort(s, i+1, r);
    }
}

Order::Order(const Order& b)
{
    _time = b._time;
    for (int i = 0; i < 3; i++)
    {
        pollution_number[i] = b.pollution_number[i];
        x[i] = b.x[i];
        forward = b.forward;
        ordertype = b.ordertype;
    }
}

Order& Order::operator=(const Order& b)
{
    _time = b._time;
    for (int i = 0; i < 3; i++)
    {
        pollution_number[i] = b.pollution_number[i];
        x[i] = b.x[i];
        forward = b.forward;
        ordertype = b.ordertype;
    }
}

void Order::swap(Order& b)
{
    Order tmp(*this);
    *this = b;
    b = tmp;
}

Order::Order()
{
    for (int i = 0; i < 3; i++)
    {
        pollution_number[i] = -100;
        x[i].SetValue(-100, -100);
    }
}
