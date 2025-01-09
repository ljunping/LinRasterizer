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
    virtual void run(DrawCallContext* context, int vert_index);
};

class LightShadowMapVertShader : public VertShader
{
    INIT_TYPE(LightShadowMapVertShader, VertShader)
    void run(DrawCallContext* context, int vert_index) override;
}
;


class GlobalRayTraceVertShader : public VertShader
{
    INIT_TYPE(GlobalRayTraceVertShader, VertShader)
    void run(DrawCallContext* context, int vert_index) override;
}
;


#endif //VERTSHADER_H
