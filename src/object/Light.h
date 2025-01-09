//
// Created by Lin on 2024/12/23.
//

#ifndef LIGHT_H
#define LIGHT_H
#include "Color.h"
#include "Component.h"
#include "Context.h"
#include "DrawCallNodeComponent.h"
#include "L_math.h"
#include "Texture.h"

class LightFragShader;
struct Fragment;
struct GPUCmd;
class DrawCallContext;
class Context;
class Camera;
class Light : public DrawCallNodeComponent
{
    INIT_TYPE(Light, DrawCallNodeComponent)
public:
    Vec3 c_light;
    float max_distance{};
    virtual float calculate_shadow(Camera* camera, const L_MATH::Vec<float, 3>& world_pos);
    virtual void on_camera_remove(Camera*);
    void on_create() override;
    void on_delete() override;
    virtual bool intersect(const Vec3& pos,const Vec3& dir);
    virtual Vec3 calculate_radiance(Vec3 pos);
};
struct LightDrawCallInfo
{
    Frustum frustum;
    Light* light;
    Color* shadow_map{};
    Fragment* fragment_map{};
    float* depth_buff{};
    int shadow_map_width{};
    int shadow_map_height{};
    Mat44 proj;
    Mat44 view;
    Mat44 pv;
};
class SpotLight : public Light
{
    INIT_TYPE(SpotLight, Light)
    std::unordered_map<Camera*, LightDrawCallInfo> light_draw_infos;
    bool rebuild_shadow_map = false;
    bool save_images = false;
    Mat44 global_mat;
    float cos_angle = 0;
    Transform* sun_node{};
public:
    bool pause_update = false;
    bool need_update() const override;
    float const_bias = 0.05f;
    float bias = 0.01f;
    float radius = 0.01f;
    ~SpotLight() override;
    float calculate_shadow(Camera* camera, const L_MATH::Vec<float, 3>& world_pos) override;
    void on_camera_remove(Camera*) override;
    void update_light_shadow_params(Camera* camera, std::vector<std::pair<Mesh*,Mat44>>& meshes);
    float angle{};
    void collect_draw_call_cmds(Camera* camera, std::vector<GPUCmds>& d_cmds) override;
    void update(float time) override;
    Vec3 calculate_radiance(L_MATH::Vec<float, 3> pos) override;
    bool intersect(const L_MATH::Vec<float, 3>& pos, const L_MATH::Vec<float, 3>& dir) override;
}
;

class PointLight : public Light
{
    INIT_TYPE(PointLight, Light)
    void collect_draw_call_cmds(Camera* camera,std::vector<GPUCmds>& d_cmds) override;


};

class DirectionalLight : public Light
{
    INIT_TYPE(DirectionalLight, Light)
    void collect_draw_call_cmds(Camera* camera,std::vector<GPUCmds>& d_cmds) override;
};


class LightManager : public ObjectManger<Light>
{
public:
    int get_light_count();
};

#endif //LIGHT_H
