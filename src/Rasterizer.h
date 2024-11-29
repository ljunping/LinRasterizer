//
// Created by Lin on 2024/11/15.
//

#ifndef RASTERIZER_H
#define RASTERIZER_H

#pragma once

#include <condition_variable>
#include <thread>
#include <X11/Xlib.h>

#include "Camera.h"
#include "Color.h"
#include "JobSystem.h"
#include "RasterizerJob.h"
#include "TrianglePrimitive.h"

struct RayCasterWork;
struct Work;
class WindowHandle;
class LuaFragShader;
struct FragShaderWork;
struct ClearWork;

struct  Fragment
{
    TrianglePrimitive* triangle = nullptr;
    Vec3 alpha;
    Vec3 frag_coord;
    Vec2 resolution;
    VertexAttribute vertex_attribute;
};

struct RayCasterResult
{
    TrianglePrimitive* triangle = nullptr;
    Vec3 alpha;
    float z;
};


class Rasterizer
{
private:
    bool enable_depth = true;
    LuaFragShader* frag_shader{};
    int frame_begin_job_id = 0;
    int frame_cur_max_job_id = 0;
    void update_frame_job_id(int job_group_id);

public:
    int w, h;
    int thread_tile_size=64;
    std::vector<Fragment> fragment_map;
    std::vector<float> depth_buff;
    std::vector<RayCasterWork> ray_cast_works;
    std::vector<FragShaderWork> frag_shader_works;
    std::vector<ClearWork> clear_works;
    Scene* scene{};
    Camera* camera{};
    Rasterizer(Scene* scene, Camera* camera, LuaFragShader*);
    ~Rasterizer();
    int ray_cast_scene();
    int render_fragment(Color*);
    int clear_color(Color* data, Color color);
    int raster_scene();
    void wait_render_finish();
    bool is_render_job_finish(int job_group_id) const;
    void draw_begin();
    void draw_end();
    void on_window_resize(int w, int h);
};




#endif //RASTERIZER_H
