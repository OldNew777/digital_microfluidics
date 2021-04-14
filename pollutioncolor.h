#ifndef POLLUTIONCOLOR_H
#define POLLUTIONCOLOR_H

#include <QColor>

class PollutionColor
{
    int size;
    QColor * color;

public:
    PollutionColor(int _size = 100);
    ~PollutionColor();

    QColor operator[](int num);
};

#endif // POLLUTIONCOLOR_H
