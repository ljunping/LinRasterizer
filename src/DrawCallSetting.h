//
// Created by Lin on 2024/12/30.
//

#ifndef DRAWCALLSETTING_H
#define DRAWCALLSETTING_H
#include "Color.h"


struct DrawCallContextSetting
{
    bool build_bvh = false;
    bool enable_ray_cast = false;
    bool enable_edge = false;
    bool enable_mipmap = true;
    int msaa_factor = 1;
    bool enable_depth = true;
    bool enable_depth_write = true;
    Color edge_color = GREEN;
    int background_color = BLACK;
    float shadow_distance = 10;
    bool enable_shadow = true;
    bool enable_light_interpolation = true;
};

#endif //DRAWCALLSETTING_H
