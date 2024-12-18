//
// Created by Lin on 2024/12/4.
//

#ifndef CONTEXT_H
#define CONTEXT_H

#include <tuple>
#include <vector>
#include "Color.h"

struct Fragment;
struct RenderPass;
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

class Context
{

public:
    WindowHandle* window_handle;
    TransformManager* transform_manager;
    RenderManager* render_manager;
    ComponentUpdateManager* component_update_manager;
    Transform* root;
    int scene_node_count;
    int cur_frame_buffer_id;
    bool build_bvh;
    bool enable_ray_cast;
    std::vector<std::tuple<void(*)(Context* context, Color* buff, void* data), void*>> after_scene_render_funcs;
    std::vector<RenderPass> render_passes;
    std::vector<Camera*> cameras;
    int render_pass_index = 0;
    int background_color=BLACK;
    explicit Context(WindowHandle* handle);
    void init();
    RenderPass* current_render_pass();
    void on_key_event(int key);
    void update(float t);
    void on_window_resize(int w, int h);
    void render();
    void main_loop();
    Color* get_frame_buffer(int frame_id);
    void render_after_scene(Color* buff);
    void clear_color();
    void register_after_scene_render_func(void(* func)(Context* ctx, Color* buff, void* data),void *data);
    void unregister_after_scene_render_func(void(* func)(Context* ctx, Color* buff, void* data),
                                            void* data);
};
extern Context* current_context;

void set_current_ctx(Context* ctx);

Context* get_current_ctx();


#endif //CONTEXT_H
