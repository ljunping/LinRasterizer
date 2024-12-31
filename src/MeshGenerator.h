//
// Created by Lin on 2024/12/30.
//
#ifndef  MESHGENERATOR_H

#define MESHGENERATOR_H
#include "Color.h"

class Mesh;
class Context;

struct DrawLineInfo
{
    int x0, y0, x1, y1;
    Color color;
};

void draw_line(Context* ctx, Color* buff, void* data);

Mesh* generate_sphere(float radius, int stacks, int slices);

Mesh* generate_quad();

Mesh* generate_tri();


#endif //MESHGENERATOR_H
