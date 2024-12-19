//
// Created by Lin on 2024/12/4.
//
#include "WindowHandle.h"
#include "Context.h"
#include <iostream>
#include "Camera.h"
#include "EventSystem.h"
#include "MeshRender.h"
#include "RasterizerJob.h"
#include "Transform.h"

const int target_fps = 60; // 目标帧率
const int frame_duration = 1000 / target_fps; // 每帧持续时间（毫秒）
Context* current_context;

Context::Context(WindowHandle* handle): window_handle(handle), transform_manager(nullptr), render_manager(nullptr),
                    component_update_manager(nullptr),
                    root(nullptr),
                    scene_node_count(0),
                    cur_frame_buffer_id(0),
                    build_bvh(false),
                    enable_ray_cast(false)
{

}

void Context::init()
{
    transform_manager = new TransformManager();
    render_manager = new RenderManager();
    component_update_manager = new ComponentUpdateManager();
    root = CREATE_OBJECT_BY_TYPE(Transform);
}



RenderPass* Context::current_render_pass()
{
    return &render_passes[render_pass_index];
}

void Context::on_key_event(KeySym key)
{
    switch (key)
    {
    case XK_1:
        {
            this->enable_ray_cast = !this->enable_ray_cast;
            this->build_bvh = this->enable_ray_cast;
        }
    }
}

void Context::update(float t)
{
    this->component_update_manager->update(t);
}

void Context::on_window_resize(int w, int h)
{
    std::cout << "Context::on_window_resize(" << w << ", " << h << ")" << std::endl;
    EventSystem::dispatch_event(WindowSizeChange, w, h);
}

void Context::set_msaa_factor(int msaa_factor)
{
    if (msaa_factor != this->msaa_factor)
    {
        this->msaa_factor = msaa_factor;
        EventSystem::dispatch_event(MSAAUpdate);
    }
}

void Context::get_screen_size(int& w, int& h) const
{
    w = this->window_handle->w;
    h = this->window_handle->h;
}

void Context::render()
{
    clear_color();
    std::sort(cameras.begin(), cameras.end(), [](Camera* a, Camera* b)
    {
        return a->scene_node->get_global_pos()[2] < b->scene_node->get_global_pos()[2];
    });
    for (auto camera : cameras)
    {
        render_passes.clear();
        this->render_manager->calculate_render_pass(camera, render_passes);
        if (render_passes.empty())
        {
            continue;
        }
        this->render_pass_index = 0;
        camera->draw_begin(this);
        for (int i = 0; i < render_passes.size(); ++i)
        {
            this->render_pass_index = i;
            camera->generate_primitive(this);
            for (int j = 0; j < this->msaa_factor; ++j)
            {
                this->msaa_index = j;
                camera->clear(this);
                if (!this->enable_ray_cast)
                {
                    camera->raster_scene(this);
                }else
                {
                    camera->ray_cast_scene(this);
                }
                camera->wait_finish();
            }
        }
        camera->render_fragment(this);
        camera->draw_end(this);
    }
    this->window_handle->draw_frame_buff();

}

void Context::main_loop()
{
    auto start_time = std::chrono::high_resolution_clock::now();
    auto last_time = start_time;
    auto now_time = last_time;
    float t = 0;
    while (true)
    {
        PERFORMANCE_DEBUG(MAIN_LOOP);
        last_time = now_time;
        now_time = std::chrono::high_resolution_clock::now();
        t = std::chrono::duration_cast<std::chrono::milliseconds>(now_time - last_time).count() * 1.0f / 1000.f;
        window_handle->event_loop();
        update(t);
        render();
        // 计算渲染时间并休眠以限制帧率
        if (t < frame_duration)
        {
            // std::this_thread::sleep_for(std::chrono::milliseconds(frame_duration - elapsed_time));
        }
    }
}

Color* Context::get_frame_buffer(int frame_id) const
{
    return this->window_handle->get_frame_buff(frame_id);
}

void Context::render_after_scene(Color* buff)
{
    for (auto after_scene_render_func : after_scene_render_funcs)
    {
        auto func = std::get<0>(after_scene_render_func);
        auto data = std::get<1>(after_scene_render_func);
        func(this, buff, data);
    }

}

void Context::clear_color()
{
    auto job_group = JOB_SYSTEM.create_job_group(0);
    JOB_SYSTEM.alloc_jobs(job_group, 0,
                          0, this->window_handle->w * this->window_handle->h,
                          this, clear_context_execute, default_complete);

    JOB_SYSTEM.submit_job_group(job_group);
    JOB_SYSTEM.wait_job_group_finish(job_group);
}

void Context::register_after_scene_render_func(void(* func)(Context* ctx, Color* buff, void* data), void* data)
{
    this->after_scene_render_funcs.emplace_back(func, data);
}

void Context::unregister_after_scene_render_func(void(* func)(Context* ctx, Color* buff, void* data), void* data)
{
    for (auto begin = after_scene_render_funcs.begin(); begin != after_scene_render_funcs.end(); ++begin)
    {
        if (*std::get<0>(*begin) == func && std::get<1>(*begin) == data)
        {
            after_scene_render_funcs.erase(begin);
            return;
        }
    }
}

