/**********************************************************************
 * Some stuff to handle chessboard
 **********************************************************************/
#ifndef _BOARD_H__
#define _BOARD_H__
#include "vector.h"
#include <iostream>
const int grid_row = 6;
const int grid_col = 8;
const int grid_len = 3;
const Point corner = {-10, -7, -25};

bool intersect_board(Point eye_pos, Vector ray, Point *hit);
bool check_in_board(Point point);
RGB_float color_board(Point point);
Vector norm_board(Point point);

bool intersect_board(Point eye_pos, Vector ray, Point *hit)
{
    // float k = (D - A * eye_pos.x - B * eye_pos.y - C * eye_pos.z) / (A * ray.x + B * ray.y + C * ray.z);
    float k = (corner.y - eye_pos.y) / ray.y;
    Point point = Point{eye_pos.x + k * ray.x,
                        eye_pos.y + k * ray.y,
                        eye_pos.z + k * ray.z};
    // std::cout << point.x << " " << point.y << " " << point.z << std::endl;
    if (k > 0 && check_in_board(point))
    {
        // set hit
        hit->x = point.x;
        hit->y = point.y;
        hit->z = point.z;
        return true;
    }
    else
    {
        return false;
    }
}

bool check_in_board(Point point)
{
    return (point.x >= corner.x && point.x <= corner.x + grid_col * grid_len) &&
           (point.z >= corner.z && point.z <= corner.z + grid_row * grid_len);
}

RGB_float color_board(Point point)
{
    if ((int((point.x - corner.x) / grid_len) + int((point.z - corner.z) / grid_len)) % 2 == 0)
    {
        return RGB_float{0.8, 0.9, 0.7};
    }
    else
    {
        return RGB_float{0.3, 0.2, 0.1};
    }
}

Vector norm_board(Point point)
{
    return Vector{0, 1, 0};
}

#endif