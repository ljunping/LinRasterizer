//
// Created by Lin on 2024/11/15.
//

#ifndef CAMERA_H
#define CAMERA_H
#include "Color.h"
#include "Component.h"
#include "FragShader.h"
#include "Light.h"
#include "L_math.h"
struct GPUCmds;
class Context;
class TrianglePrimitive;

#define GET_CAMERA(id) (Camera*)TypeFactory::get_type_inst_by_inst_id(id)

class Camera : public DrawCallNodeComponent
{
    INIT_TYPE(Camera, DrawCallNodeComponent)
    bool is_proj{};
    void on_create() override;
    void on_delete() override;
    Camera(float near, float far, float fov, float ratio, bool isProj);
    Mat44 view_mat;
    Mat44 proj_mat;
public:
    ~Camera() override;
    int w;
    int h;
    int render_layer = 1;
    bool solid_color = false;
    Fragment* fragment_map{};
    float* depth_buff{};
    Color background_color = BLACK;
    bool enable_ray_cast = false;
    float near, far, fov, ratio;
    const Mat44& get_view_mat() const;
    const Mat44& get_proj_mat() const;
    void update_view_mat() ;
    void update_proj_mat() ;
    bool is_render_layer(int sort_layer) const;
    void collect_draw_call_cmds(std::vector<GPUCmds>& d_cmds) override;
};

class CamaraManager : public ObjectManger<Camera>
{
public:
    void get_cameras(std::vector<Camera*>& cameras)
    {
        for (auto object : objects)
        {
            cameras.emplace_back(object);
        }
    }
    Camera* get_camera(int render_layer);
};


#endif //CAMERA_H
