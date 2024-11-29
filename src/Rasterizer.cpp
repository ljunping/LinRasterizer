//
// Created by Lin on 2024/11/15.
//

#include "Rasterizer.h"
#include <iostream>
#include <thread>
#include "Color.h"
#include "debug.h"
#include "JobSystem.h"
#include "RasterizerJob.h"


struct RayCasterWork;

void Rasterizer::on_window_resize(int w, int h)
{
    this->w = w;
    this->h = h;
    std::cout << "Rasterizer::on_window_resize(" << w << ", " << h << ")" << std::endl;
    depth_buff.resize(w * h);
    fragment_map.resize(w * h);
}


void Rasterizer::update_frame_job_id(int job_group_id)
{
    if (frame_begin_job_id == 0)
    {
        frame_begin_job_id = job_group_id;
        frame_cur_max_job_id = job_group_id;
    }
    frame_cur_max_job_id = std::max(frame_cur_max_job_id, job_group_id);
}

Rasterizer::Rasterizer(Scene* scene, Camera* camera, LuaFragShader* frag_shader): scene(scene), camera(camera),
                                                                                  frag_shader(frag_shader)
{
}


Rasterizer::~Rasterizer()
{
}

int Rasterizer::ray_cast_scene()
{
    int block_count_x = (w + thread_tile_size - 1) / thread_tile_size;
    int block_count_y = (h + thread_tile_size - 1) / thread_tile_size;
    if (ray_cast_works.size()!=block_count_x * block_count_y)
    {
        ray_cast_works.resize(block_count_x * block_count_y);
    }
    for (int i = 0; i < block_count_x; ++i)
    {
        for (int j = 0; j < block_count_y; ++j)
        {
            auto& work = ray_cast_works[i * block_count_y + j];
            work.base.TYPE = RAY_CAST_FRAG;
            work.base.block_x = i;
            work.base.block_y = j;
        }
    }
    auto job_group = JOB_SYSTEM.create_job_group(frame_cur_max_job_id);
    JOB_SYSTEM.alloc_jobs(job_group, 0,
                          0, ray_cast_works.size(),
                          this, ray_cast_frag_execute,ray_cast_frag_complete);
    JOB_SYSTEM.submit_job_group(job_group);
    update_frame_job_id(job_group);
    return job_group;
}

int Rasterizer::render_fragment(Color* buffer)
{
    int block_count_x = (w + thread_tile_size - 1) / thread_tile_size;
    int block_count_y = (h + thread_tile_size - 1) / thread_tile_size;
    if (frag_shader_works.size()!=block_count_x * block_count_y)
    {
        frag_shader_works.resize(block_count_x * block_count_y);
    }
    for (int i = 0; i < block_count_x; ++i)
    {
        for (int j = 0; j < block_count_y; ++j)
        {
            auto& work = frag_shader_works[i * block_count_y + j];
            work.base.TYPE = FRAG_SHADER;
            work.base.block_x = i;
            work.base.block_y = j;
            work.buffer = buffer;
            work.frag_shader = frag_shader;
        }
    }
    auto prepare_fence = JOB_SYSTEM.create_job_group(frame_cur_max_job_id);
    JOB_SYSTEM.alloc_jobs(prepare_fence, 0,
                          0, frag_shader_works.size(),
                          this, prepare_frag_shader_execute, default_complete);

    JOB_SYSTEM.submit_job_group(prepare_fence);

    auto job_group = JOB_SYSTEM.create_job_group(prepare_fence);
    JOB_SYSTEM.alloc_jobs(job_group, 0,
                          0, frag_shader_works.size(),
                          this, run_frag_shader_execute,run_frag_shader_complete);

    JOB_SYSTEM.submit_job_group(job_group);
    update_frame_job_id(job_group);
    return job_group;
}


int Rasterizer::clear_color(Color* data, Color color)
{
    int block_count_x = (w + thread_tile_size - 1) / thread_tile_size;
    int block_count_y = (h + thread_tile_size - 1) / thread_tile_size;
    if (clear_works.size()!=block_count_x * block_count_y)
    {
        clear_works.resize(block_count_x * block_count_y);
    }
    for (int i = 0; i < block_count_x; ++i)
    {
        for (int j = 0; j < block_count_y; ++j)
        {
            ClearWork* work = &clear_works[i * block_count_y + j];
            work->base.block_x = i;
            work->base.block_y = j;
            work->buffer = data;
            work->color = color;
        }
    }
    auto job_group = JOB_SYSTEM.create_job_group(frame_cur_max_job_id);

    JOB_SYSTEM.alloc_jobs(job_group, 0,
                          0, clear_works.size(),
                          this, clear_execute,clear_complete);

    JOB_SYSTEM.submit_job_group(job_group);
    update_frame_job_id(job_group);
    return job_group;
}


int Rasterizer::raster_scene()
{
    auto& proj_triangle_list = camera->proj_triangles;
    auto tri_count = proj_triangle_list.size();
    if (tri_count == 0)
    {
        return 0;
    }
    auto job_group = JOB_SYSTEM.create_job_group(frame_cur_max_job_id);
    JOB_SYSTEM.alloc_jobs(job_group, 0,
                          0, proj_triangle_list.size(),
                          this, rast_tri_execute,rast_tri_complete);
    JOB_SYSTEM.submit_job_group(job_group);
    update_frame_job_id(job_group);
    return job_group;
}

void Rasterizer::wait_render_finish()
{
    JOB_SYSTEM.wait_job_group_finish(frame_cur_max_job_id);
}

bool Rasterizer::is_render_job_finish(int job_group_id) const
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

void Rasterizer::draw_begin()
{
    camera->update_proj_triangle_list();

    frame_cur_max_job_id = 0;
    frame_begin_job_id = 0;
}

void Rasterizer::draw_end()
{
    wait_render_finish();
}






