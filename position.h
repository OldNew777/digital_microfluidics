#ifndef POSITION_H
#define POSITION_H

#include <cmath>

class Position
{
public:
    int _x, _y;

    Position(){}
    Position(const int &x, const int &y)
    {
        _x = x;
        _y = y;
    }
    Position(const Position& b)
    {
        _x = b._x;
        _y = b._y;
    }

    Position& operator=(const Position& b)
    {
        _x = b._x;
        _y = b._y;
    }
    bool operator==(const Position& b)
    {
        return (_x == b._x && _y == b._y);
    }
    bool operator!=(const Position& b)
    {
        return (_x != b._x || _y != b._y);
    }

    bool legal(Position b)
    {
        return (abs(_x - b._x) > 1 || abs(_y - b._y) > 1);
    }
    void SetValue(const int &x, const int &y)
    {
        _x = x;
        _y = y;
    }

    void swap(Position & b)
    {
        int f = _x;
        _x = b._x;
        b._x = f;

        f = _y;
        _y = b._y;
        b._y = f;
    }
};

#endif // POSITION_H
