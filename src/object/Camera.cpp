//
// Created by Lin on 2024/11/15.
//

#include "Camera.h"
#include "EventSystem.h"
#include "MeshRender.h"
#include "RasterizerJob.h"
#include "Transform.h"
#include "DrawCallContext.h"
#include "DrawCallSetting.h"
#include "GPU.h"

static void window_resize(Camera* camera)
{
    auto current_ctx = get_current_ctx();
    int w, h;
    current_ctx->get_screen_size(w, h);
    camera->w = w;
    camera->h = h;
    if (camera->fragment_map!=nullptr)
    {
        free(camera->fragment_map);
        free(camera->depth_buff);
    }
    camera->fragment_map = static_cast<Fragment*>(malloc(
        sizeof(Fragment) * w * h * current_ctx->setting.msaa_factor * current_ctx->setting.msaa_factor));
    camera->depth_buff = static_cast<float*>(malloc(
        sizeof(float) * w * h * current_ctx->setting.msaa_factor * current_ctx->setting.msaa_factor));
    camera->ratio = w / (float)h;
}

static void window_resize(EventParam& param, void* camera)
{
    window_resize((Camera*)camera);
}


Camera::Camera(float near, float far, float fov, float ratio, bool isproj):
    near(near), far(far),
    fov(fov), ratio(ratio), is_proj(isproj)
{
}



Camera::~Camera()
{
    free(this->fragment_map);
    free(this->depth_buff);
}


void Camera::on_create()
{
    DrawCallNodeComponent::on_create();
    EventSystem::register_listener(WindowSizeChange, window_resize, (void*)this);
    EventSystem::register_listener(MSAAUpdate, window_resize, (void*)this);
    auto current_ctx = get_current_ctx();
    window_resize(this);
    current_ctx->camara_manager->on_create_obj(this);
}

void Camera::on_delete()
{
    auto current_ctx = get_current_ctx();
    current_ctx->camara_manager->on_delete_obj(this);
    EventSystem::unregister_listener(WindowSizeChange, window_resize, this);
    EventSystem::unregister_listener(MSAAUpdate, window_resize, (void*)this);
    DrawCallNodeComponent::on_delete();
}


const L_MATH::Mat<float, 4, 4>& Camera::get_view_mat() const
{
    return view_mat;
}

const L_MATH::Mat<float, 4, 4>& Camera::get_proj_mat() const
{
    return proj_mat;
}

void Camera::update_view_mat()
{
    auto local_to_global_mat = this->scene_node->get_local_to_global_mat();
    L_MATH::invert_trs_mat(local_to_global_mat, view_mat);
}

void Camera::update_proj_mat()
{
    proj_mat = (is_proj
                    ? L_MATH::project(near, far, fov, ratio)
                    : L_MATH::ortho(near, far, fov, ratio));
}

bool Camera::is_render_layer(int sort_layer) const
{
    return this->render_layer & sort_layer;
}
static bool compare_transparent_render_node(const RenderNode& a, const RenderNode& b)
{
    return a.model_matrix[3][2] < b.model_matrix[3][2];
}
void Camera::collect_draw_call_cmds(std::vector<GPUCmds>& d_cmds)
{
    update_proj_mat();
    update_view_mat();
    auto ctx = get_current_ctx();
    DrawCallContext draw_call_context{};
    draw_call_context.fragment_map = this->fragment_map;
    draw_call_context.depth_buff = this->depth_buff;
    draw_call_context.frame_buff = ctx->get_frame_buffer(0);
    draw_call_context.view_matrix = this->get_view_mat();
    draw_call_context.proj_matrix = this->get_proj_mat();
    draw_call_context.setting = ctx->setting;
    draw_call_context.view_world_pos = this->scene_node->get_global_pos();
    draw_call_context.setting.background_color = this->background_color;
    if (this->solid_color)
    {
        d_cmds.emplace_back(draw_call_context, 3, CLEAR_FRAG, CLEAR_DEPTH, CLEAR_FRAME_BUFF);
    }else
    {
        d_cmds.emplace_back(draw_call_context, 2, CLEAR_FRAG, CLEAR_DEPTH);
    }
    std::vector<RenderNode> render_nodes;
    ctx->render_node_manager->collection_render_node(this, render_nodes, false);
    if (!render_nodes.empty())
    {
        d_cmds.emplace_back(draw_call_context, DRAW);
        std::ranges::sort(render_nodes, less_compare_render_node);
        for (auto render_node : render_nodes)
        {
            if (!d_cmds.back().context.try_add_render_node(render_node))
            {
                d_cmds.emplace_back(draw_call_context, DRAW);
                d_cmds.back().context.try_add_render_node(render_node);
            }
        }
        render_nodes.clear();
    }
    ctx->render_node_manager->collection_render_node(this, render_nodes, true);
    if (!render_nodes.empty())
    {
        std::ranges::sort(render_nodes, compare_transparent_render_node);
        draw_call_context.setting.enable_depth_write = false;
        for (auto render_node : render_nodes)
        {
            d_cmds.emplace_back(draw_call_context, DRAW);
            d_cmds.back().context.try_add_render_node(render_node);
        }
    }
}


Camera* CamaraManager::get_camera(int render_layer)
{
    auto& cameras = this->get_objects();
    for (auto camera : cameras)
    {
        if (camera->is_render_layer(render_layer))
        {
            return camera;
        }
    }
    return nullptr;
}










