//
// Created by Lin on 2024/11/15.
//

#ifndef CAMERA_H
#define CAMERA_H
#include "Color.h"
#include "FragShader.h"
#include "Light.h"
#include "L_math.h"
struct GPUCmds;
class Context;
struct TrianglePrimitive;

#define GET_CAMERA(id) (Camera*)TypeFactory::get_type_inst_by_inst_id(id)

class Camera : public DrawCallNodeComponent
{
    INIT_TYPE(Camera, DrawCallNodeComponent)
    bool is_proj{};
    void on_create() override;
    void on_delete() override;
    Camera(float near, float far, float fov, float ratio, bool);
    Mat44 view_mat;
    Mat44 proj_mat;
    Mat44 proj_view;
    bool dirty = true;
    void prepare_context(Context*& ctx, DrawCallContext& draw_call_context);
    void prepare_global_draw_call(std::vector<GPUCmds>& d_cmds, Context* ctx, DrawCallContext& draw_call_context,
                                  std::vector<RenderNode>& render_nodes);
    void prepare_untransparent_draw_call(std::vector<GPUCmds>& d_cmds, DrawCallContext& draw_call_context,
                                         std::vector<RenderNode>& render_nodes);
    void prepare_transparent_draw_call(std::vector<GPUCmds>& d_cmds, DrawCallContext& draw_call_context,
                                       std::vector<RenderNode>& render_node_transparent);
    void split_render_node(Context* ctx, std::vector<RenderNode>& render_nodes,
                           std::vector<RenderNode>& render_node_transparent);

    SHARE_PTR<GlobalRayTraceVertShader> global_shader;

public:
    ~Camera() override;
    Frustum view_frustum;
    int render_layer = 1;
    bool solid_color = false;
    Fragment* fragment_map{};
    float* depth_buff{};
    Color background_color = BLACK;
    Mat44& get_view_mat();
    Mat44& get_proj_mat();
    void set_dirty();
    void clear_dirty() ;
    Frustum get_shadow_view_frustum(Context* ctx) const;
    Mat44& get_proj_view_mat();
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
