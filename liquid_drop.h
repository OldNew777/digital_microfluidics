#ifndef LIQUID_DROP_H
#define LIQUID_DROP_H

#include "position.h"

enum Shape
{
    Circle = 1,

    Ellipse = 2
};

struct LiquidDrop
{
    int pollution_number = -100;
	
    Shape shape = Circle;
    Position interaction_target;

    void clear()
    {
        pollution_number = -100;
        shape = Circle;
    }


};

bool illegal(const LiquidDrop & a, const LiquidDrop & b, const int & ax, const int & ay, const int & bx, const int & by);

#endif // LIQUID_DROP_H
