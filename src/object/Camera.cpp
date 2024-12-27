//
// Created by Lin on 2024/11/15.
//

#include "Camera.h"
#include "EventSystem.h"
#include "MeshRender.h"
#include "RasterizerJob.h"
#include "Transform.h"
#include "DrawCallContext.h"

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


Camera::Camera(float near, float far, float fov, float ratio, bool isproj): Component(), near(near), far(far),
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

int Camera::clear(Context* ctx)
{
    update_proj_mat();
    update_view_mat();
    auto job_group = JOB_SYSTEM.create_job_group(0);

    JOB_SYSTEM.alloc_jobs(job_group, 0,
                          0, w * h, new std::tuple<Context*, Camera*>(ctx, this)
                          , clear_camera_execute, clear_camera_complete);
    JOB_SYSTEM.submit_job_group(job_group);
    return job_group;
}







