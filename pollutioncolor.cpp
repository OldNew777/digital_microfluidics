#include "pollutioncolor.h"

PollutionColor::PollutionColor(int _size)
{
    size = std::max(_size, 100);
    color = new QColor[size];
    for (int i = 0; i < size; i++)
        color[i].setRgb(rand() % 255, rand() % 255, rand() % 255);
}

PollutionColor::~PollutionColor()
{
    delete [] color;
}

QColor PollutionColor::operator[](int num)
{
    if (num > size - 1)
    {
        int new_size = num << 1;
        QColor * tmp = new QColor[new_size];
        for (int i = 0; i < size; i++)
            tmp[i] = color[i];
        delete [] color;
        color = tmp;
        for (int i = size + 1; i < new_size; i++)
            color[i].setRgb(rand() % 255, rand() % 255, rand() % 255);
        size = new_size;
    }
    return color[num];
}
