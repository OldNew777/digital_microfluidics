#include "liquid_drop.h"

bool illegal(const LiquidDrop & a, const LiquidDrop & b, const int & ax, const int & ay, const int & bx, const int & by)
{
    //任意格子没有液滴在上面，则合法
    if (a.pollution_number < 0 || b.pollution_number < 0 || a.pollution_number == b.pollution_number)
        return false;

    //任意液滴不是椭圆态，则二者不可能发生互动，不合法
    if (a.shape != Ellipse || b.shape != Ellipse)
        return true;

    //椭圆态，二者互动
    if (a.interaction_target._x == bx && a.interaction_target._y == by &&
            b.interaction_target._x == ax && b.interaction_target._y == ay)
        return false;

    //椭圆态，但二者未互动
    return true;
}
