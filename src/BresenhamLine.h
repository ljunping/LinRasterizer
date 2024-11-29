//
// Created by Lin on 2024/11/21.
//

#ifndef BRESEHAMLINE_H
#define BRESEHAMLINE_H
#include <cmath>


class BresenhamLine {

    public:
    int x1, y1, x2, y2, dx, dy, stepY, stepX, cur_x, cur_y, p;
    BresenhamLine(): y1(0), x2(0), y2(0), dy(0), cur_y(0), p(0)
    {
    }

    BresenhamLine(int x1, int y1, int x2, int y2): x1(x1), y1(y1), x2(x2), y2(y2)
    {
        dx = x2 - x1;
        dy = y2 - y1;
        stepX = dx >= 0 ? 1 : -1;
        stepY = dy >= 0 ? 1 : -1;;
        dx = ::abs(dx);
        dy = ::abs(dy);
        cur_x = x1;
        cur_y = y1;
        p = dx > dy ? 2 * dy - dx : 2 * dx - dy;
    };
    bool has_next()
    {
        if (abs(cur_x - x1) >= dx && abs(cur_y - y1) >= dy)
        {
            return false;
        }
        return true;
    }

    bool end_with(float x, float y)
    {

        if (abs(cur_x - x1) >= dx && abs(cur_y - y1) >= dy)
        {
            return false;
        }
        return true;
    }

    bool next_point(int& nx, int& ny, int& move_x, int& move_y)
    {
        if (!has_next())
        {
            return false;
        }
        move_x = 0;
        move_y = 0;
        if (dx > dy)
        {
            // |m| < 1
            ny = cur_y;
            nx = cur_x + stepX;
            move_x = stepX;
            if (p > 0)
            {
                ny += stepY;
                move_y = stepY;
                p -= 2 * dx;
            }
            p += 2 * dy;
        }
        else
        {
            // |m| >= 1
            nx = cur_x;
            ny = cur_y + stepY;
            move_y = stepY;
            if (p > 0)
            {
                nx += stepX;
                move_x = stepX;
                p -= 2 * dy;
            }
            p += 2 * dx;
        }
        cur_x = nx;
        cur_y = ny;
        return true;
    }
};



#endif //BRESEHAMLINE_H
