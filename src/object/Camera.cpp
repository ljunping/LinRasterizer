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
#include "Mesh.h"
#include "VertShader.h"



static void window_resize(Camera* camera)
{
    auto current_ctx = get_current_ctx();
    int w, h;
    current_ctx->get_screen_size(w, h);
    if (camera->fragment_map!=nullptr)
    {
        free(camera->fragment_map);
        free(camera->depth_buff);
    }
    camera->fragment_map = static_cast<Fragment*>(malloc(
        sizeof(Fragment) * w * h * current_ctx->setting.msaa_factor));
    camera->depth_buff = static_cast<float*>(malloc(
        sizeof(float) * w * h * current_ctx->setting.msaa_factor));
    camera->view_frustum.aspect_ratio = w / static_cast<float>(h);
    camera->set_dirty();

}

static void window_resize(EventParam& param, void* camera)
{
    window_resize((Camera*)camera);
}


Camera::Camera(float near, float far, float fov, float ratio, bool isproj): is_proj(isproj),
                                                                            view_frustum(near, far, fov, ratio)
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
    for (auto object : current_ctx->light_manager->get_objects())
    {
        object->on_camera_remove(this);
    }
    DrawCallNodeComponent::on_delete();
}


L_MATH::Mat<float, 4, 4>& Camera::get_view_mat()
{
    clear_dirty();
    return view_mat;
}

 L_MATH::Mat<float, 4, 4>& Camera::get_proj_mat()
{
    clear_dirty();
    return proj_mat;
}

void Camera::set_dirty()
{
    dirty = true;
}

Frustum Camera::get_shadow_view_frustum(Context* ctx) const
{
    auto shadow_distance = ctx->setting.shadow_distance;
    return {view_frustum.near, std::min(shadow_distance, view_frustum.far), view_frustum.fov,
                   view_frustum.aspect_ratio};

}

L_MATH::Mat<float, 4, 4>& Camera::get_proj_view_mat()
{
    clear_dirty();
    return proj_view;
}


void Camera::clear_dirty()
{
    if (dirty)
    {
        dirty = false;
        view_mat = invert_trs_mat(this->scene_node->get_local_to_global_mat());
        proj_mat = (is_proj
                        ? view_frustum.get_proj_mat()
                        : L_MATH::ortho(view_frustum.near, view_frustum.far, view_frustum.fov,
                                        view_frustum.aspect_ratio));
        proj_view = proj_mat * view_mat;
    }
}



bool Camera::is_render_layer(int sort_layer) const
{
    return this->render_layer & sort_layer;
}
static bool compare_transparent_render_node(const RenderNode& a, const RenderNode& b)
{
    return a.model_matrix[3][2] < b.model_matrix[3][2];
}

void Camera::prepare_context(Context*& ctx, DrawCallContext& draw_call_context)
{
    ctx = get_current_ctx();
    draw_call_context.fragment_map = this->fragment_map;
    draw_call_context.depth_buff = this->depth_buff;
    draw_call_context.frame_buff = ctx->get_frame_buffer(0);
    draw_call_context.view_matrix = this->get_view_mat();
    draw_call_context.proj_matrix = this->get_proj_mat();
    draw_call_context.inv_view_matrix = this->scene_node->get_local_to_global_mat();
    draw_call_context.setting = ctx->setting;
    draw_call_context.view_world_pos = this->scene_node->get_global_pos();
    draw_call_context.setting.background_color = this->background_color;
    draw_call_context.camera = this;
    draw_call_context.host = this;
    draw_call_context.view_frustum = view_frustum;
}

void Camera::prepare_global_draw_call(std::vector<GPUCmds>& d_cmds, Context* ctx, DrawCallContext& draw_call_context,
                                      std::vector<RenderNode>& render_nodes)
{
    d_cmds.emplace_back(draw_call_context, DRAW);
    auto& d_cmd = d_cmds.back();
    for (auto& render_node : render_nodes)
    {
        d_cmd.context.add_render_node(render_node);
    }
    global_shader = Resource::copy_resource(Resource::get_or_create_default_resource<GlobalRayTraceVertShader>());
    d_cmd.context.vert_shader = global_shader;
    d_cmd.context.setting.enable_light_interpolation = false;
    d_cmd.context.setting.build_bvh = true;
}

void Camera::prepare_untransparent_draw_call(std::vector<GPUCmds>& d_cmds, DrawCallContext& draw_call_context, std::vector<RenderNode>& render_nodes)
{
    if (render_nodes.empty())
    {
        return;
    }
    std::ranges::sort(render_nodes, less_compare_render_node);
    int pre_index = -1;
    for (int i = 0; i < render_nodes.size(); ++i)
    {
        auto& render_node = render_nodes[i];
        auto& mesh = render_node.mesh;
        if (!mesh)
        {
            continue;
        }
        Mat44 vm = view_mat * render_node.model_matrix;
        if (!this->view_frustum.box_in_frustum(mesh->get_box(), vm))
        {
            continue;
        }
        if (pre_index == -1 || !equal_compare_render_node(render_nodes[pre_index], render_node))
        {
            d_cmds.emplace_back(draw_call_context, DRAW);
            d_cmds.back().context.set_mesh_render_params(render_node);
            d_cmds.back().context.add_render_node( render_node);
            pre_index = i;
        }
        else
        {
            d_cmds.back().context.add_render_node(render_node);
        }
    }
}

void Camera::prepare_transparent_draw_call(std::vector<GPUCmds>& d_cmds, DrawCallContext& draw_call_context,
                                           std::vector<RenderNode>& render_node_transparent)
{
    if (render_node_transparent.empty())
    {
        return;
    }
    std::ranges::sort(render_node_transparent, compare_transparent_render_node);
    draw_call_context.setting.enable_depth_write = false;
    for (auto& render_node : render_node_transparent)
    {
        auto& mesh = render_node.mesh;
        if (!mesh)
        {
            continue;
        }
        Mat44 vm = view_mat * render_node.model_matrix;
        if (!this->view_frustum.box_in_frustum(mesh->get_box(), vm))
        {
            continue;
        }
        d_cmds.emplace_back(draw_call_context, DRAW);
        auto& call_context = d_cmds.back().context;
        call_context.set_mesh_render_params(render_node);
        call_context.add_render_node(render_node);
    }
}

void Camera::split_render_node(Context* ctx, std::vector<RenderNode>& render_nodes, std::vector<RenderNode>& render_node_transparent )
{
    std::vector<RenderNode> render_nodes_all;
    ctx->render_node_manager->collection_render_node(this, render_nodes_all);
    for (auto& nodes_all : render_nodes_all)
    {
        if (nodes_all.transparent)
        {
            render_node_transparent.emplace_back(nodes_all);
        }
        else
        {
            render_nodes.emplace_back(nodes_all);
        }
    }
}

void Camera::collect_draw_call_cmds(std::vector<GPUCmds>& d_cmds)
{
    clear_dirty();
    Context* ctx;
    DrawCallContext draw_call_context;
    prepare_context(ctx, draw_call_context);
    if (this->solid_color)
    {
        d_cmds.emplace_back(draw_call_context, 3, CLEAR_FRAG, CLEAR_DEPTH, CLEAR_FRAME_BUFF);
    }else
    {
        d_cmds.emplace_back(draw_call_context, 2, CLEAR_FRAG, CLEAR_DEPTH);
    }
    std::vector<RenderNode> render_nodes;
    std::vector<RenderNode> render_node_transparent;
    split_render_node(ctx, render_nodes, render_node_transparent);
    if (draw_call_context.setting.enable_global_path_trace)
    {
        prepare_global_draw_call(d_cmds, ctx, draw_call_context, render_nodes);
        return;
    }
    prepare_untransparent_draw_call(d_cmds, draw_call_context, render_nodes);
    prepare_transparent_draw_call(d_cmds, draw_call_context, render_node_transparent);
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










