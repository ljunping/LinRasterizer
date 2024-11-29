//
// Created by Lin on 2024/11/20.
//
#include "RasterizerJob.h"
#include "BresenhamLine.h"
#include "PODPool.h"
#include "LuaFragShader.h"
void ray_cast_frag_execute(int data_begin, int data_end,  void* global_data)
{
    Rasterizer* rasterizer = (Rasterizer*)(global_data);
    int thread_tile_size = rasterizer->thread_tile_size;
    int w = rasterizer->w;
    int h = rasterizer->h;
    RayCasterResult result;
    for (int idx = data_begin; idx < data_end; idx++)
    {
        auto work = &rasterizer->ray_cast_works[idx];
        auto ox = (int)::fmax(work->base.block_x * thread_tile_size, 0);
        auto oy =(int) ::fmax(work->base.block_y * thread_tile_size, 0);
        auto sx = (int)::fmin((work->base.block_x + 1) * (thread_tile_size), w);
        auto sy = (int)::fmin((work->base.block_y + 1) * (thread_tile_size), h);
        for (int i = oy; i < sy; ++i)
        {
            float y = 2.f * i / h - 1;
            for (int j = ox; j < sx; ++j)
            {
                float x = 2.f * j / w - 1;
                if (!ray_caster(rasterizer->camera, x, y, &result))
                {
                    continue;
                }
                add_fragment(*result.triangle, rasterizer, result.alpha, i, j);
            }
        }
    }

}

void ray_cast_frag_complete(int data_begin, int data_end, void* global_data)
{

}

void clear_execute(int data_begin, int data_end, void* global_data)
{
    Rasterizer* rasterizer = (Rasterizer*)global_data;
    int thread_tile_size = rasterizer->thread_tile_size;
    int w = rasterizer->w;
    int h = rasterizer->h;
    for (int idx = data_begin; idx < data_end; idx++)
    {
        auto work = &rasterizer->clear_works[idx];
        auto ox = (int)::fmax(work->base.block_x * thread_tile_size, 0);
        auto oy =(int) ::fmax(work->base.block_y * thread_tile_size, 0);
        auto sx = (int)::fmin((work->base.block_x + 1) * (thread_tile_size), w);
        auto sy = (int)::fmin((work->base.block_y + 1) * (thread_tile_size), h);
        for (int i = oy; i < sy; ++i)
        {
            for (int j = ox; j < sx; ++j)
            {
                work->buffer[i * w + j] = work->color;
                rasterizer->depth_buff[i * w + j] = 1.f;
                rasterizer->fragment_map[i * w + j].triangle = nullptr;
            }
        }
    }
}

void clear_complete(int data_begin, int data_end, void* global_data)
{

}

void run_frag_shader_execute(int data_begin, int data_end, void* global_data)
{
    Rasterizer* rasterizer = (Rasterizer*)global_data;
    int thread_tile_size = rasterizer->thread_tile_size;
    int w = rasterizer->w;
    int h = rasterizer->h;
    for (int idx = data_begin; idx < data_end; ++idx)
    {
        FragShaderWork* work = &rasterizer->frag_shader_works[idx];
        auto &lua_frag_shader = *work->frag_shader;
        auto sx = (int)::fmin((work->base.block_x + 1) * (thread_tile_size), w);
        auto sy = (int)::fmin((work->base.block_y + 1) * (thread_tile_size), h);
        auto ox = (int)::fmax(work->base.block_x * thread_tile_size, 0);
        auto oy =(int) ::fmax(work->base.block_y * thread_tile_size, 0);
        lua_frag_shader.ox = ox;
        lua_frag_shader.oy = oy;
        lua_frag_shader.sx = sx;
        lua_frag_shader.sy = sy;
        lua_frag_shader.width = rasterizer->w;
        lua_frag_shader.height = rasterizer->h;
        lua_frag_shader.fragment_map = &rasterizer->fragment_map;

        for (int i = oy; i < sy; ++i)
        {
            for (int j = ox; j < sx; ++j)
            {
                auto fragment = &rasterizer->fragment_map[i * w + j];
                auto tri = fragment->triangle;
                if (tri)
                {
                    tri->vert[0].attributes->create_vert_attribute(tri->vert[0], tri->vert[1], tri->vert[2],
                                                                   fragment->alpha,
                                                                   fragment->vertex_attribute);
                    work->buffer[(h - i - 1) * w + j] = lua_frag_shader.run(i * w + j);
                }
            }
        }
    }
}

void prepare_frag_shader_execute(int data_begin, int data_end, void* global_data)
{
    Rasterizer* rasterizer = (Rasterizer*)global_data;
    int thread_tile_size = rasterizer->thread_tile_size;
    int w = rasterizer->w;
    int h = rasterizer->h;
    for (int idx = data_begin; idx < data_end; ++idx)
    {
        FragShaderWork* work = &rasterizer->frag_shader_works[idx];
        auto &lua_frag_shader = *work->frag_shader;
        auto sx = (int)::fmin((work->base.block_x + 1) * (thread_tile_size), w);
        auto sy = (int)::fmin((work->base.block_y + 1) * (thread_tile_size), h);
        auto ox = (int)::fmax(work->base.block_x * thread_tile_size, 0);
        auto oy =(int) ::fmax(work->base.block_y * thread_tile_size, 0);
        lua_frag_shader.ox = ox;
        lua_frag_shader.oy = oy;
        lua_frag_shader.sx = sx;
        lua_frag_shader.sy = sy;
        lua_frag_shader.width = rasterizer->w;
        lua_frag_shader.height = rasterizer->h;
        lua_frag_shader.fragment_map = &rasterizer->fragment_map;

        for (int i = oy; i < sy; ++i)
        {
            for (int j = ox; j < sx; ++j)
            {
                auto fragment = &rasterizer->fragment_map[i * w + j];
                auto tri = fragment->triangle;
                if (tri)
                {
                    tri->vert[0].attributes->create_vert_attribute(tri->vert[0], tri->vert[1], tri->vert[2],
                                                                   fragment->alpha,
                                                                   fragment->vertex_attribute);
                }
            }
        }
    }
}


void run_frag_shader_complete(int data_begin, int data_end, void* global_data)
{

}

void rast_tri_execute(int data_begin, int data_end, void* global_data)
{
    auto rasterizer = (Rasterizer*)global_data;
    auto& triangle_primitives = rasterizer->camera->proj_triangles;
    for (int i = data_begin; i < data_end; ++i)
    {
        auto& triangle_primitive = triangle_primitives[i];
        if (triangle_primitive.discard || triangle_primitive.inv_cross_dir_z < 0)
        {
            continue;
        }
        rast_tri(triangle_primitive, rasterizer);
    }
}

void rast_tri_complete(int data_begin, int data_end, void* global_data)
{

}

void mid_filter_execute(int data_begin, int data_end, void* global_data)
{

}

void mid_filter_complete(int data_begin, int data_end, void* global_data)
{
}

void execute_mvp_break_huge_triangle(int data_begin, int data_end, void* global_data)
{
    Camera* camera = (Camera*)global_data;
    Mat44& mvp = camera->thread_mvp;
    auto& triangles = camera->scene->get_model_triangle_list();
    auto& triangle_primitives = camera->proj_triangles;
    for (int i = data_begin; i < data_end; ++i)
    {
        if (!triangles[i].is_remove())
        {
            triangle_primitives[i].discard = false;
            triangle_primitives[i] = *triangles[i].data;
            triangle_primitives[i].update(mvp);
            if(triangle_primitives[i].area()>HUGE_TRIANGLE_THR&&HUB_TRIANGLE_BREAK_COUNT_MAX>0)
            {
                std::lock_guard<std::mutex> lock(camera->proj_triangles_mutex);
                int break_count = HUB_TRIANGLE_BREAK_COUNT_MAX;
                std::vector<TrianglePrimitive> result;
                break_huge_triangle(triangle_primitives[i], HUGE_TRIANGLE_THR, result, break_count);
                triangle_primitives.insert(triangle_primitives.end(), result.begin(), result.end());
            }
        }
        else
        {
            triangle_primitives[i].discard = true;
        }
    }
}

void complete_mvp_break_huge_triangle(int data_begin, int data_end, void* global_data)
{

}


bool add_fragment(TrianglePrimitive& tri, Rasterizer* rasterizer,L_MATH::Vec<float, 3>& alpha, int i, int j)
{
    L_MATH::clamp(alpha,Vec3::ZERO,Vec3::ONE);
    auto w = rasterizer->w;
    int index = i * w + j;
    auto st_z = tri.v[0][2] * alpha[0] + tri.v[1][2] * alpha[1] + tri.v[2][2] * alpha[2];
    if (rasterizer->depth_buff[index] < st_z)
    {
        return false;
    }
    auto alpha_inv_w = 1 / L_MATH::dot(alpha, tri.inv_w);
    rasterizer->depth_buff[index] = st_z;
    alpha *= tri.inv_w * alpha_inv_w;
    auto real_z = -1 * alpha_inv_w;
    auto &fragment = rasterizer->fragment_map[index];
    fragment.triangle = &tri;
    fragment.frag_coord = Vec3{(float)j, (float)i, real_z};
    fragment.alpha = alpha;
    fragment.resolution[0] = w;
    fragment.resolution[1] = rasterizer->h;

    return true;
}

bool ray_caster(Camera* camera, float si, float sj, RayCasterResult* result)
{
    // TIME_RUN_BEGIN()
    auto& proj_triangle_list = camera->proj_triangles;
    Vec3 sv = Vec3({si, sj, -1});
    float max_z = -camera->near;
    bool _is_ray_caster = false;
    Vec3 alpha;
    for (int i = 0; i < proj_triangle_list.size(); ++i)
    {
        auto& triangle = proj_triangle_list[i];
        if (triangle.discard)
        {
            continue;
        }
        if (triangle.inv_cross_dir_z < 0)
        {
            continue;
        }
        if (!triangle.box.inside_2d(sv))
        {
            continue;
        }
        triangle.barycentric(sv, alpha);
        if (triangle.is_same_sign(alpha))
        {
            auto z = -1.f / L_MATH::dot(alpha, triangle.inv_w);
            alpha = alpha * triangle.inv_w / L_MATH::dot(alpha, triangle.inv_w);
            if (z < max_z && z > -camera->far)
            {
                result->triangle = &triangle;
                result->alpha = alpha;
                result->z = z;
                _is_ray_caster = true;
            }
        }
    }
    return _is_ray_caster;
}

void rast_tri(TrianglePrimitive& tri, Rasterizer* rasterizer)
{
    auto h = rasterizer->h;
    auto w = rasterizer->w;
    tri.clip();
    auto& clip_vertices = tri.clip_vertices;
    auto& alpha_array = tri.clip_vertices_alpha;
    if (clip_vertices.empty())
    {
        return;
    }
    int vert_count = clip_vertices.size();
    auto clip_vertices_int = new int[vert_count][2];
    int l_line = 0, top_line;
    for (int i = 0; i < alpha_array.size(); ++i)
    {
        vert_view_transform(rasterizer->camera->thread_mvp, w, h, tri.vert, alpha_array[i], clip_vertices_int[i][0],
                            clip_vertices_int[i][1]);

    }
    int l_y = clip_vertices_int[0][1],
        m_y = clip_vertices_int[0][1],
        m_x = clip_vertices_int[0][0],
        l_x = clip_vertices_int[0][0];

    int top_left_v = 0;

    for (int i = 0; i < clip_vertices.size(); ++i)
    {
        if (clip_vertices_int[i][1] < l_y)
        {
            l_y = clip_vertices_int[i][1];
            l_x = clip_vertices_int[i][0];
            l_line = i;
        }
        if (clip_vertices_int[i][1] > m_y)
        {
            m_y = clip_vertices_int[i][1];
            m_x = clip_vertices_int[i][0];
            top_left_v = i;
        }
        if (clip_vertices_int[i][1] == l_y && clip_vertices_int[i][0] < l_x)
        {
            l_y = clip_vertices_int[i][1];
            l_x = clip_vertices_int[i][0];
            l_line = i;
        }
        if (clip_vertices_int[i][1] == m_y && clip_vertices_int[i][0] < m_x)
        {
            m_y = clip_vertices_int[i][1];
            m_x = clip_vertices_int[i][0];
            top_left_v = i;
        }
    }
    bool is_left_line[vert_count];
    std::fill(is_left_line, is_left_line + vert_count, false);
    int _temp = l_line;
    _temp = (_temp - 1 + vert_count) % vert_count;
    while (_temp != top_left_v)
    {
        is_left_line[_temp] = true;
        _temp = (_temp - 1 + vert_count) % vert_count;
    }
    is_left_line[top_left_v] = true;

    top_line = top_left_v;
    int lx, ly, rx, ry, lmx, lmy, rmx, rmy;
    bool next_left = true, next_right = true, jump_line_l, jump_line_r;
    int r_line = (l_line - 1 + vert_count) % vert_count;

    BresenhamLine l_bresenham_line = BresenhamLine(clip_vertices_int[l_line][0], clip_vertices_int[l_line][1],
                                     clip_vertices_int[(l_line + 1) % vert_count][0],
                                     clip_vertices_int[(l_line + 1) % vert_count][1]);

    BresenhamLine r_bresenham_line = BresenhamLine(clip_vertices_int[(r_line + 1) % vert_count][0],
                                     clip_vertices_int[(r_line + 1) % vert_count][1], clip_vertices_int[r_line][0],
                                     clip_vertices_int[r_line][1]);
    Vec3 l_alpha = alpha_array[l_line];
    Vec3 r_alpha = alpha_array[(r_line + 1) % vert_count];
    Vec3 st_l, st_r;
    while (l_line != r_line)
    {
        int l0 = -1;
        jump_line_l = clip_vertices_int[l_line][1] == clip_vertices_int[(l_line + 1) % vert_count][1];
        jump_line_r = clip_vertices_int[r_line][1] == clip_vertices_int[(r_line + 1) % vert_count][1];
        //水平顶边绘制
        if ((jump_line_l &&(l_line == top_line || (l_line + 1) % vert_count == top_line)))
        {
            l0 = l_line;
        }
        if (jump_line_r && (r_line == top_line || (r_line + 1) % vert_count == top_line))
        {
            l0 = r_line;
        }
        {
            if(l0==-1)
            {
                goto next;
            }
            auto l0y = clip_vertices_int[l0][1];
            auto l0x1 = clip_vertices_int[l0][0];
            auto l0x2 = clip_vertices_int[(l0 + 1) % vert_count][0];
            if (l0x1==l0x2)
            {
                goto next;
            }
            int step = l0x1 > l0x2 ? -1 : 1;
            auto& alpha_l0 = alpha_array[l0];
            auto& l_alpha1 = alpha_array[(l0 + 1) % vert_count];
            float t = 0;
            Vec3 alpha;
            for (int i = l0x1; i != l0x2; i += step)
            {
                t = (i - l0x1) * 1.0 / (l0x2 - l0x1);
                alpha = alpha_l0 * (1 - t) + l_alpha1 * t;
                add_fragment(tri, rasterizer, alpha, l0y, i);

            }
        }
    next:
        if (jump_line_l || !l_bresenham_line.has_next())
        {
            l_line = (l_line + 1) % vert_count;
            l_alpha = alpha_array[l_line];
            l_bresenham_line = BresenhamLine(clip_vertices_int[l_line][0], clip_vertices_int[l_line][1],
                                             clip_vertices_int[(l_line + 1) % vert_count][0],
                                             clip_vertices_int[(l_line + 1) % vert_count][1]);
            continue;
        }
        if (jump_line_r || !r_bresenham_line.has_next())
        {
            r_line = (r_line - 1 + vert_count) % vert_count;
            r_alpha = alpha_array[(r_line + 1) % vert_count];
            r_bresenham_line = BresenhamLine(clip_vertices_int[(r_line + 1) % vert_count][0],
                                             clip_vertices_int[(r_line + 1) % vert_count][1],
                                             clip_vertices_int[r_line][0],
                                             clip_vertices_int[r_line][1]);
            continue;
        }

        if (next_left)
        {
            l_bresenham_line.next_point(lx, ly, lmx, lmy);
            // float t = (ly - l_bresenham_line.y1) * 1.0f / (l_bresenham_line.dy);
            // l_alpha = alpha_array[l_line] * (1 - t) + alpha_array[(l_line + 1) % vert_count] * t;
            //
            invert_view_transform(w, h, lx, ly, st_l[0], st_l[1]);
            tri.barycentric(st_l, l_alpha);

            if (is_left_line[l_line])
            {
                add_fragment(tri, rasterizer, l_alpha, ly, lx);
            }

        }

        if (next_right)
        {
            r_bresenham_line.next_point(rx, ry, rmx, rmy);
            // float t = (ry - r_bresenham_line.y1) * 1.0f / (r_bresenham_line.dy);
            // r_alpha = alpha_array[(r_line + 1) % vert_count] * (1 - t) + alpha_array[r_line] * t;

            invert_view_transform(w, h, rx, ry, st_r[0], st_r[1]);
            tri.barycentric(st_r, r_alpha);

            if (is_left_line[r_line])
            {
                add_fragment(tri, rasterizer, r_alpha, ry, rx);
            }
        }
        next_right = false;
        next_left = false;
        if (ly == ry)
        {
            next_left = true;
            next_right = true;
            if (rx == lx)
            {
                continue;
            }
            int step = rx > lx ? 1 : -1;
            float t = 0;
            auto alpha = l_alpha;
            for (int i = lx + step; i != rx; i += step)
            {
                // t = (i - lx) * 1.0 / (rx - lx);
                // alpha = l_alpha * (1 - t) + r_alpha * t;
                invert_view_transform(w, h, i, ry, st_r[0], st_r[1]);
                tri.barycentric(st_r, alpha);
                add_fragment(tri, rasterizer, alpha, ry, i);
            }
        }
        else if (ly < ry)
        {
            next_left = true;
        }
        else
        {
            next_right = true;
        }
    }
    delete[] clip_vertices_int;
}
// 第一次调用必须tri.area() > area_thr && count > 0为true
void break_huge_triangle(TrianglePrimitive& tri, float area_thr, std::vector<TrianglePrimitive>& result,
                         int& count)
{
    if (tri.area() > area_thr && count > 0)
    {
        tri.discard = true;
        --count;
        Vec3 alphas[6];
        int vert_index[12] = {0, 1, 5, 1, 2, 3, 1, 3, 5, 5, 3, 4};
        alphas[0] = Vec3{1, 0, 0};
        alphas[1] = Vec3{0.5, 0.5, 0};
        alphas[2] = Vec3{0, 1, 0};
        alphas[3] = Vec3{0, 0.5, 0.5};
        alphas[4] = Vec3{0, 0, 1};
        alphas[5] = Vec3{0.5, 0, 0.5};
        for (int i = 0; i < 12; i = i + 3)
        {
            Vec3 alpha0 = alphas[vert_index[i]];
            Vec3 alpha1 = alphas[vert_index[i + 1]];
            Vec3 alpha2 = alphas[vert_index[i + 2]];
            auto tri_result = Pool<TrianglePrimitive>::POOL.create();

            tri_result->v[0] = tri.v[0] * alpha0[0] + tri.v[1] * alpha0[1] + tri.v[2] * alpha0[2];
            tri_result->v[1] = tri.v[0] * alpha1[0] + tri.v[1] * alpha1[1] + tri.v[2] * alpha1[2];
            tri_result->v[2] = tri.v[0] * alpha2[0] + tri.v[1] * alpha2[1] + tri.v[2] * alpha2[2];
            tri_result->inv_w = Vec3::ONE;
#if DEBUG
            tri_result->id = tri.id;
#endif

            auto attributes = tri.vert[0].attributes;

            attributes->create_vert_attribute(tri.vert[0], tri.vert[1], tri.vert[2], alpha0, tri_result->vert[0]);

            attributes->create_vert_attribute(tri.vert[0], tri.vert[1], tri.vert[2], alpha1, tri_result->vert[1]);

            attributes->create_vert_attribute(tri.vert[0], tri.vert[1], tri.vert[2], alpha2, tri_result->vert[2]);
            tri_result->update_param();
            break_huge_triangle(*tri_result, area_thr, result, count);
        }
    }
    else
    {
        result.emplace_back(tri);
        Pool<TrianglePrimitive>::POOL.recycle(&tri);
    }
}









