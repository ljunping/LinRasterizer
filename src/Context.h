//
// Created by Lin on 2024/12/4.
//

#ifndef CONTEXT_H
#define CONTEXT_H

#include <tuple>
#include <vector>
#include <X11/Xlib.h>
#include "Color.h"
#include "DrawCallSetting.h"

class Light;
class LightManager;
class CamaraManager;
struct Fragment;
struct DrawCallContext;
class Transform;
class ComponentUpdateManager;
class RenderNodeManager;
class WindowHandle;
class Context;
class TransformManager;
class Mesh;
class FragShader;
class Texture;
class Material;
class Camera;



class Context
{
    friend class WindowHandle;
    void on_key_event(XEvent& event);
    void update_scene(float t);
    void check_setting_change();
    void render_scene();
    void set_draw_call_parameters(DrawCallContext& draw_call_context);
    void render_after_scene(Color* buff);
    std::vector<std::tuple<void(*)(Context* context, Color* buff, void* data), void*>> after_scene_render_funcs;
    DrawCallContextSetting compare_setting;
    WindowHandle* window_handle{};
    void init();
public:
    TransformManager* transform_manager{};
    RenderNodeManager* render_node_manager{};
    CamaraManager* camara_manager{};
    ComponentUpdateManager* component_update_manager{};
    LightManager* light_manager{};
    DrawCallContextSetting setting;
    explicit Context(WindowHandle* handle);
    void get_screen_size(int& w, int& h) const;
    void main_loop();
    Color* get_frame_buffer(int frame_id) const;
    void register_after_scene_render_func(void(* func)(Context* ctx, Color* buff, void* data),void *data);
    void unregister_after_scene_render_func(void(* func)(Context* ctx, Color* buff, void* data),
                                            void* data);
};
extern Context* current_context;

inline void set_current_ctx(Context* ctx)
{
    current_context = ctx;
}

inline Context* get_current_ctx()
{
    return current_context;
}


#endif //CONTEXT_H
