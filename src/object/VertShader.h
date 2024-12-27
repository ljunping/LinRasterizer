//
// Created by Lin on 2024/12/26.
//

#ifndef VERTSHADER_H
#define VERTSHADER_H
#include "Resource.h"


class Mesh;
struct DrawCallContext;
struct VertexInterpolation;

class VertShader : public Resource
{
    INIT_TYPE(VertShader, Resource)
public:
    void run(DrawCallContext* context, int vert_index);
};



#endif //VERTSHADER_H
