//
// Created by Lin on 2024/12/30.
//

#ifndef DRAWCALLNODECOMPONENT_H
#define DRAWCALLNODECOMPONENT_H
#include "Component.h"
class Camera;
struct GPUCmds;
class DrawCallContext;


class DrawCallNodeComponent : public Component
{
    INIT_TYPE(DrawCallNodeComponent, Component)

public:

    virtual void collect_draw_call_cmds(std::vector<GPUCmds>& d_cmds)
    {
    };

    virtual void collect_draw_call_cmds(Camera* camera, std::vector<GPUCmds>& d_cmds)
    {

    };
}
;


#endif //DRAWCALLNODECOMPONENT_H
