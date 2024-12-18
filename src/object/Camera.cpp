//
// Created by Lin on 2024/11/15.
//

#include "Camera.h"

#include "EventSystem.h"
#include "JobSystem.h"
#include "MeshRender.h"
#include "RasterizerJob.h"
#include "TrianglePrimitive.h"
#include "WindowHandle.h"


static void window_resize(int w, int h, Camera* camera)
{
    ((Camera*)camera)->fragment_map.resize(w * h);
    ((Camera*)camera)->depth_buff.resize(w * h);
    camera->ratio = w / (float)h;
}

static void window_resize(EventParam& param, void* camera)
{
    window_resize(param.m_n1, param.m_n2, (Camera*)camera);
}

void Camera::update_view_mat()
{
    view_mat = L_MATH::look_at(look_dir, L_MATH::UP);
}
void Camera::update_projection_mat()
{
    projection_mat = ( is_proj
                         ? L_MATH::project(near, far, fov, ratio)
                         : L_MATH::ortho(near, far, fov, ratio));

    projection_mat = L_MATH::translate(pos * -1) * projection_mat;
}

Camera::Camera(float near, float far, float fov, float ratio, bool isproj): Component(), near(near), far(far),
                                                                            fov(fov), ratio(ratio), is_proj(isproj)
{
    look_dir = L_MATH::FORWARD;
    pos = Vec3::ZERO;
    update_view_mat();
    update_projection_mat();
}


void Camera::get_proj_triangle_list(Context* ctx)
{
    update_view_mat();
    update_projection_mat();
    auto pass = ctx->current_render_pass();
    std::vector<std::pair<int,Mat44>>& meshes = pass->meshes;
    std::vector<std::tuple<RenderPass*,Mat44>> data(meshes.size());

    int pri_size = 0;
    for (int i = 0; i < meshes.size(); ++i)
    {
        auto& mesh_pair = meshes[i];
        auto& mesh = mesh_pair.first;
        auto& model_mat = mesh_pair.second;
        Mat44 mvp = projection_mat * view_mat * model_mat;
        pri_size += Resource::get_resource<Mesh>(mesh)->tri_count();
        data[i] = std::make_tuple(pass, mvp);
    }
    pass->assign_triangle_primitives(pri_size);
    int final_job_group_id = 0;
    int begin = 0, end = 0;
    for (int i = 0; i < meshes.size(); ++i)
    {
        auto& mesh_pair = meshes[i];
        auto& mesh = mesh_pair.first;
        end += Resource::get_resource<Mesh>(mesh)->tri_count();
        auto job_group = JOB_SYSTEM.create_job_group(0);
        final_job_group_id = job_group;
        JOB_SYSTEM.alloc_jobs(job_group,
                              0,
                              begin,
                              end,
                              data.data() + i,
                              execute_mvp,
                              default_complete);
        JOB_SYSTEM.submit_job_group(job_group);
        begin += Resource::get_resource<Mesh>(mesh)->tri_count();
    }

    JOB_SYSTEM.wait_job_group_finish(final_job_group_id);
    if (ctx->build_bvh)
    {
        pass->build_bvh_tree();
    }
}

void Camera::update_frame_job_id(int job_group_id)
{
    if (frame_begin_job_id == 0)
    {
        frame_begin_job_id = job_group_id;
        frame_cur_max_job_id = job_group_id;
    }
    frame_cur_max_job_id = std::max(frame_cur_max_job_id, job_group_id);
}



void Camera::on_create()
{
    Component::on_create();
    EventSystem::register_listener(WindowSizeChange, window_resize, (void*)this);
    auto current_ctx = get_current_ctx();
    window_resize(current_ctx->window_handle->w, current_ctx->window_handle->h, this);
    current_ctx->cameras.push_back(this);
}

void Camera::on_delete()
{
    auto& cameras = get_current_ctx()->cameras;
    cameras.erase(std::find(cameras.begin(), cameras.end(), this));
    EventSystem::unregister_listener(WindowSizeChange, window_resize, this);
    Component::on_delete();
}


void Camera::render_pass(Context* ctx)
{
    this->get_proj_triangle_list(ctx);
    this->raster_scene(ctx);
    this->render_fragment(ctx);
}


bool Camera::is_render_layer(int sort_layer) const
{
    return this->render_layer & sort_layer;
}


int Camera::ray_cast_scene(Context* ctx)
{
    auto job_group = JOB_SYSTEM.create_job_group(frame_cur_max_job_id);
    JOB_SYSTEM.alloc_jobs(job_group, 0,
                          0, ctx->window_handle->w * ctx->window_handle->h,
                          ctx, ray_cast_frag_execute, ray_cast_frag_complete);
    JOB_SYSTEM.submit_job_group(job_group);
    update_frame_job_id(job_group);
    return job_group;
}

int Camera::render_fragment(Context* ctx)
{
    auto current_render_pass = ctx->current_render_pass();
    auto frag_shader = Resource::get_resource<FragShader>(current_render_pass->pass_node.frag_shader);
    frag_shader->begin_render_pass(ctx, current_render_pass);
    auto prepare_fence = JOB_SYSTEM.create_job_group(frame_cur_max_job_id);
    JOB_SYSTEM.alloc_jobs(prepare_fence, 0,
                          0, ctx->window_handle->w * ctx->window_handle->h,
                          ctx, prepare_frag_shader_execute, default_complete);


    JOB_SYSTEM.submit_job_group(prepare_fence);

    auto job_group = JOB_SYSTEM.create_job_group(prepare_fence);
    JOB_SYSTEM.alloc_jobs(job_group, 0,
                          0, ctx->window_handle->w * ctx->window_handle->h,
                          frag_shader, run_frag_shader_execute, run_frag_shader_complete);

    JOB_SYSTEM.submit_job_group(job_group);
    update_frame_job_id(job_group);
    return job_group;
}


int Camera::clear_color(Context* ctx)
{
    auto job_group = JOB_SYSTEM.create_job_group(frame_cur_max_job_id);

    JOB_SYSTEM.alloc_jobs(job_group, 0,
                          0, ctx->window_handle->w * ctx->window_handle->h,
                          ctx, clear_camera_execute, clear_complete);

    JOB_SYSTEM.submit_job_group(job_group);
    update_frame_job_id(job_group);
    return job_group;
}


int Camera::raster_scene(Context* ctx)
{
    if (ctx->enable_ray_cast)
    {
        return ray_cast_scene(ctx);
    }
    auto render_pass = ctx->current_render_pass();
    auto tri_count = render_pass->primitives.size();
    if (tri_count == 0)
    {
        return 0;
    }
    auto job_group = JOB_SYSTEM.create_job_group(frame_cur_max_job_id);
    JOB_SYSTEM.alloc_jobs(job_group, 0,
                          0, render_pass->primitives.size(),
                          ctx, rast_tri_execute, rast_tri_complete);
    JOB_SYSTEM.submit_job_group(job_group);
    update_frame_job_id(job_group);
    return job_group;
}

void Camera::wait_render_finish()
{
    JOB_SYSTEM.wait_job_group_finish(frame_cur_max_job_id);
}

bool Camera::is_render_job_finish(int job_group_id) const
{
    if (job_group_id < frame_begin_job_id)
    {
        return true;
    }
    if (job_group_id > frame_cur_max_job_id)
    {
        return false;
    }
    return JOB_SYSTEM.is_job_group_finish(job_group_id);
}

void Camera::draw_begin(Context* ctx)
{
    frame_cur_max_job_id = 0;
    frame_begin_job_id = 0;
    clear_color(ctx);
}

void Camera::draw_end(Context* ctx)
{
    wait_render_finish();
    auto current_render_pass = ctx->current_render_pass();
    auto frag_shader = Resource::get_resource<FragShader>(current_render_pass->pass_node.frag_shader);
    frag_shader->end_render_pass(ctx, current_render_pass);
}





