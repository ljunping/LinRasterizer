//
// Created by Lin on 2024/12/4.
//

#ifndef CONTEXT_H
#define CONTEXT_H

#include <tuple>
#include <vector>
#include <X11/Xlib.h>
#include "Color.h"

class LightManager;
class CamaraManager;
struct Fragment;
struct DrawCallContext;
class Transform;
class ComponentUpdateManager;
class RenderManager;
class WindowHandle;
class Context;
class TransformManager;
class Mesh;
class FragShader;
class Texture;
class Material;
class Camera;

struct ContextSetting
{
    bool build_bvh = false;
    bool enable_ray_cast = false;
    bool enable_edge = false;
    bool enable_mipmap = true;
    int msaa_factor = 1;
    bool enable_depth = true;
    bool enable_depth_write = true;
    Color edge_color = GREEN;
    int background_color = BLACK;
};

class Context
{
    friend class WindowHandle;
    void render();
    void render_camera(Camera* camera);
    void on_key_event(XEvent& event);
    void update_scene(float t);
    void check_setting_change();
    void render_after_scene(Color* buff);
    void clear_color();
    std::vector<std::tuple<void(*)(Context* context, Color* buff, void* data), void*>> after_scene_render_funcs;
    std::vector<DrawCallContext> draw_calls;
    ContextSetting compare_setting;
    WindowHandle* window_handle;
    void init();
public:
    TransformManager* transform_manager;
    RenderManager* render_manager;
    CamaraManager* camara_manager;
    ComponentUpdateManager* component_update_manager;
    LightManager* light_manager;
    ContextSetting setting;
    explicit Context(WindowHandle* handle);
    void get_screen_size(int& w, int& h) const;
    Camera* get_camera(int render_layer) const;
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
