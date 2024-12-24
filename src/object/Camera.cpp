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


static void window_resize(Camera* camera)
{
    auto current_ctx = get_current_ctx();
    int w, h;
    current_ctx->get_screen_size(w, h);
    camera->fragment_map.resize(w * h * current_ctx->setting.msaa_factor * current_ctx->setting.msaa_factor);
    camera->depth_buff.resize(w * h * current_ctx->setting.msaa_factor * current_ctx->setting.msaa_factor);
    camera->ratio = w / (float)h;
}

static void window_resize(EventParam& param, void* camera)
{
    window_resize((Camera*)camera);
}


void Camera::update_projection_mat()
{
    projection_mat = ( is_proj
                         ? L_MATH::project(near, far, fov, ratio)
                         : L_MATH::ortho(near, far, fov, ratio));

}

Camera::Camera(float near, float far, float fov, float ratio, bool isproj): Component(), near(near), far(far),
                                                                            fov(fov), ratio(ratio), is_proj(isproj)
{
    update_projection_mat();
}


void Camera::generate_primitive(Context* ctx)
{
    update_projection_mat();
    auto pass = ctx->current_render_pass();
    std::vector<std::pair<int,Mat44>>& meshes = pass->meshes;
    std::vector<std::tuple<DrawCallData*,Mat44>> data(meshes.size());

    int pri_size = 0;
    for (int i = 0; i < meshes.size(); ++i)
    {
        auto& mesh_pair = meshes[i];
        auto& mesh = mesh_pair.first;
        auto& model_mat = mesh_pair.second;
        pri_size += Resource::get_resource<Mesh>(mesh)->tri_count();
        data[i] = std::make_tuple(pass, projection_mat * get_view_mat() * model_mat);
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
    if (ctx->setting.build_bvh)
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
    Component::on_delete();
}





bool Camera::is_render_layer(int sort_layer) const
{
    return this->render_layer & sort_layer;
}


int Camera::ray_cast_scene(Context* ctx)
{
    int w, h;
    ctx->get_screen_size(w, h);
    auto job_group = JOB_SYSTEM.create_job_group(frame_cur_max_job_id);
    JOB_SYSTEM.alloc_jobs(job_group, 0,
                          0, w * h,
                          ctx, ray_cast_frag_execute, ray_cast_frag_complete);
    JOB_SYSTEM.submit_job_group(job_group);
    update_frame_job_id(job_group);
    return job_group;
}

int Camera::render_fragment(Context* ctx)
{
    int w, h;
    ctx->get_screen_size(w, h);
    auto current_render_pass = ctx->current_render_pass();
    auto frag_shader = Resource::get_resource<FragShader>(current_render_pass->pass_node.frag_shader);
    auto prepare_fence = JOB_SYSTEM.create_job_group(frame_cur_max_job_id);
    JOB_SYSTEM.alloc_jobs(prepare_fence, 0,
                          0, w * h,
                          ctx, prepare_frag_shader_execute, default_complete);



    JOB_SYSTEM.submit_job_group(prepare_fence);

    auto job_group = JOB_SYSTEM.create_job_group(prepare_fence);
    JOB_SYSTEM.alloc_jobs(job_group, 0,
                          0, w * h,
                          frag_shader, run_frag_shader_execute, run_frag_shader_complete);

    JOB_SYSTEM.submit_job_group(job_group);
    update_frame_job_id(job_group);
    return job_group;
}


int Camera::clear(Context* ctx)
{
    int w, h;
    ctx->get_screen_size(w, h);
    auto job_group = JOB_SYSTEM.create_job_group(frame_cur_max_job_id);

    JOB_SYSTEM.alloc_jobs(job_group, 0,
                          0, w * h,
                          ctx, clear_camera_execute, clear_complete);
    JOB_SYSTEM.submit_job_group(job_group);
    update_frame_job_id(job_group);
    return job_group;
}

L_MATH::Mat<float, 4, 4> Camera::get_view_mat() const
{
    Mat44 view_mat;
    auto local_to_global_mat = this->scene_node->get_local_to_global_mat();
    L_MATH::invert_trs_mat(local_to_global_mat, view_mat);
    return view_mat;
}


int Camera::raster_scene(Context* ctx)
{
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

void Camera::wait_finish()
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
}

void Camera::draw_end(Context* ctx)
{
    wait_finish();
}







