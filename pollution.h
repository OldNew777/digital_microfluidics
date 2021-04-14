#ifndef POLLUTION_H
#define POLLUTION_H

#include "Bitmap.h"
#include "QPointF"

class Pollution
{
private:
    int num = 0;
    Bitmap source;

public:
    QPointF point[2];

    Pollution(){}

    void pollute(int number)
    {
        if (number == 0)
        {
            reset();
            return;
        }
        if (source.test(number))
            return;
        ++num;
        source.set(number);
    }
    void clean(int number)
    {
        if (source.test(number))
        {
            --num;
            source.clear(number);
        }
    }
    void reset()
    {
        num = 0;
        source.clearAll();
    }
    bool test(int number)
    {
        if (num > 0 && source.test(number))
            return true;
        return false;
    }
    int getPollution()
    {
        return num;
    }
};

#endif // POLLUTION_H
