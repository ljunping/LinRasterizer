//
// Created by Lin on 2024/12/4.
//
#include "WindowHandle.h"
#include "Context.h"
#include <iostream>
#include "Camera.h"
#include "DrawCallContext.h"
#include "EventSystem.h"
#include "GPU.h"
#include "Light.h"
#include "MeshRender.h"
#include "RasterizerJob.h"
#include "Transform.h"


const int target_fps = 60; // 目标帧率
const int frame_duration = 1000 / target_fps; // 每帧持续时间（毫秒）
Context* current_context;

Context::Context(WindowHandle* handle): window_handle(handle), transform_manager(nullptr), render_node_manager(nullptr),
                                        component_update_manager(nullptr), camara_manager(nullptr)
{

}

void Context::init()
{
    transform_manager = new TransformManager();
    render_node_manager = new RenderNodeManager();
    component_update_manager = new ComponentUpdateManager();
    camara_manager = new CamaraManager();
    light_manager = new LightManager();
}


int select_mesh_index;
void Context::on_key_event(XEvent& event)
{
    // 获取按键事件
    KeySym keysym = XLookupKeysym(&event.xkey, 0); // 获取 keysym，0 表示不使用修饰键
    switch (keysym)
    {
    case XK_1:
        {

        }
    }
}

void Context::update_scene(float t)
{
    this->component_update_manager->update(t);
}

void Context::check_setting_change()
{
    if (compare_setting.msaa_factor != this->setting.msaa_factor)
    {
        EventSystem::dispatch_event(MSAAUpdate, this->setting.msaa_factor);
        compare_setting.msaa_factor = this->setting.msaa_factor;
    }
}


void Context::get_screen_size(int& w, int& h) const
{
    w = this->window_handle->w;
    h = this->window_handle->h;
}


void Context::render_scene()
{
    std::vector<GPUCmds> cmds;
    DrawCallContext context;
    context.setting = setting;
    context.frame_buff = get_frame_buffer(0);
    cmds.emplace_back(context, CLEAR_FRAME_BUFF);
    for (auto object : light_manager->get_objects())
    {
        object->collect_draw_call_cmds(cmds);
    }
    std::vector<Camera*> cameras;
    camara_manager->get_cameras(cameras);
    std::sort(cameras.begin(), cameras.end(), [](Camera* a, Camera* b)
    {
        return a->scene_node->get_global_pos()[2] < b->scene_node->get_global_pos()[2];
    });
    for (auto camera : cameras)
    {
        camera->collect_draw_call_cmds(cmds);
    }
    GPU::begin();
    for (auto& cmd : cmds)
    {
        set_draw_call_parameters(cmd.context);
        GPU::run(cmd);
    }
    GPU::end();
    render_after_scene(this->get_frame_buffer(0));
    this->window_handle->draw_frame_buff();
}

void Context::set_draw_call_parameters(DrawCallContext& draw_call_context)
{
    draw_call_context.ctx = this;
    get_screen_size(draw_call_context.w, draw_call_context.h);
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
        check_setting_change();
        update_scene(t);
        render_scene();
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

