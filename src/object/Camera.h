//
// Created by Lin on 2024/11/15.
//

#ifndef CAMERA_H
#define CAMERA_H
#include "Color.h"
#include "Component.h"
#include "FragShader.h"
#include "L_math.h"
class Context;
class TrianglePrimitive;

#define GET_CAMERA(id) (Camera*)TypeFactory::get_type_inst_by_inst_id(id)

class Camera : public Component
{
    INIT_TYPE(Camera, Component)
    bool is_proj;
    void update_projection_mat();
    bool is_render_job_finish(int job_group_id) const;
    int frame_begin_job_id = 0;
    int frame_cur_max_job_id = 0;
    void update_frame_job_id(int job_group_id);
    void on_create() override;
    void on_delete() override;
    Camera(float near, float far, float fov, float ratio, bool isProj);
public:
    int render_layer = 1;
    bool solid_color = false;
    std::vector<Fragment> fragment_map;
    std::vector<float> depth_buff;
    Color background_color = BLACK;
    bool enable_ray_cast = false;
    Mat44 projection_mat;
    float near, far, fov, ratio;
    int render_fragment(Context* ctx);
    void draw_begin(Context* ctx);
    void draw_end(Context* ctx);
    int ray_cast_scene(Context* ctx);
    int raster_scene(Context* ctx);
    void generate_primitive(Context* ctx);
    void wait_finish();
    int clear(Context* ctx);
    Mat44 get_view_mat() const;
    bool is_render_layer(int sort_layer) const;
}

;

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
};


#endif //CAMERA_H
