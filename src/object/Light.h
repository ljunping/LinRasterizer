//
// Created by Lin on 2024/12/23.
//

#ifndef LIGHT_H
#define LIGHT_H
#include "Component.h"
#include "DrawCallNodeComponent.h"
#include "L_math.h"

struct GPUCmd;
class DrawCallContext;
class Context;
class Camera;

class Light : public DrawCallNodeComponent
{
    INIT_TYPE(Light, DrawCallNodeComponent)
public:
    Vec3 color;
    float intensity{};
    virtual float calculate_shadow(Vec3& world_pos);
    void on_create() override;
    void on_delete() override;
};
class SpotLight : public Light
{
    INIT_TYPE(SpotLight, Light)
    public:
    float max_distance{};
    Vec3 dir;
    float angle{};
    void collect_draw_call_cmds(std::vector<GPUCmds>& d_cmds) override;
};

class PointLight : public Light
{
    INIT_TYPE(PointLight, Light)
    void collect_draw_call_cmds(std::vector<GPUCmds>& d_cmds) override;


};

class DirectionalLight : public Light
{
    INIT_TYPE(DirectionalLight, Light)
    void collect_draw_call_cmds(std::vector<GPUCmds>& d_cmds) override;


};


class LightManager : public ObjectManger<Light>
{
public:
    int get_light_count();
};

#endif //LIGHT_H
