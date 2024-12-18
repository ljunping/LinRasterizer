//
// Created by Lin on 2024/11/20.
//
#include "RasterizerJob.h"
#include "BresenhamLine.h"
#include "Camera.h"
#include "PODPool.h"
#include "FragShader.h"
#include "WindowHandle.h"
#include "TrianglePrimitive.h"
#include"Context.h"
#include "Mesh.h"
#include "MeshRender.h"

void ray_cast_frag_execute(std::size_t data_begin,std::size_t data_end,  void* global_data)
{
    Context* ctx = (Context*)global_data;
    int w = ctx->window_handle->w;
    int h = ctx->window_handle->h;
    auto current_render_pass = ctx->current_render_pass();
    RayCasterResult result;
    for (int idx = data_begin; idx < data_end; idx++)
    {
        int i = idx / w, j = idx % w;
        float y,x;
        invert_view_transform(w, h, j, i, x, y);
        if (ctx->build_bvh)
        {
            if (!ray_caster_bvh(current_render_pass->pass_node.camera, current_render_pass->bvh_tree, x, y, &result))
            {
                continue;
            }
        }else
        {
            if (!ray_caster(ctx, x, y, &result))
            {
                continue;
            }
        }

        add_fragment(*result.triangle,ctx, result.alpha, i, j);
    }

}

void ray_cast_frag_complete(std::size_t data_begin,std::size_t data_end, void* global_data)
{

}

void clear_context_execute(std::size_t data_begin, std::size_t data_end, void* global_data)
{
    Context* ctx = (Context*)global_data;
    auto frame_buff = ctx->get_frame_buffer(0);
    auto background_color = ctx->background_color;
    for (int idx = data_begin; idx < data_end; idx++)
    {
        frame_buff[idx] = background_color;
    }
}


void clear_camera_execute(std::size_t data_begin,std::size_t data_end, void* global_data)
{
    Context* ctx = (Context*)global_data;
    auto current_render_pass = ctx->current_render_pass();
    auto& pass_node = current_render_pass->pass_node;
    auto background_color = pass_node.camera->background_color;
    Color* frame_buff = ctx->get_frame_buffer(pass_node.frame_buff_index);
    for (int idx = data_begin; idx < data_end; idx++)
    {
        if (pass_node.camera->solid_color)
        {
            frame_buff[idx] = background_color;
        }
        pass_node.camera->depth_buff[idx] = 1.f;
        pass_node.camera->fragment_map[idx].triangle = nullptr;
    }
}

void clear_complete(std::size_t data_begin,std::size_t data_end, void* global_data)
{

}

void run_frag_shader_execute(std::size_t data_begin,std::size_t data_end, void* global_data)
{
    FragShader* frag_shader = (FragShader*)global_data;
    Color* frame_buff = frag_shader->frame_buff;
    int w = frag_shader->width;
    int h = frag_shader->height;
    for (int idx = data_begin; idx < data_end; ++idx)
    {
        auto& fragment = (*frag_shader->fragment_map)[idx];
        int i = fragment.frag_coord[1], j = fragment.frag_coord[0];
        auto tri = fragment.triangle;
        if (tri)
        {
            frame_buff[(h - i - 1) * w + j] = frag_shader->run(i * w + j);
        }
    }
}

void prepare_frag_shader_execute(std::size_t data_begin,std::size_t data_end, void* global_data)
{
    Context* context = (Context*)global_data;
    int w = context->window_handle->w;
    int h = context->window_handle->h;
    auto current_render_pass = context->current_render_pass();
    auto& pass_node = current_render_pass->pass_node;
    for (int idx = data_begin; idx < data_end; ++idx)
    {
        auto lua_frag_shader = *Resource::get_resource<FragShader>(pass_node.frag_shader);
        lua_frag_shader.width = w;
        lua_frag_shader.height = h;
        lua_frag_shader.fragment_map = &pass_node.camera->fragment_map;
        auto fragment = &pass_node.camera->fragment_map[idx];
        auto tri = fragment->triangle;
        if (!tri)
            continue;
        tri->vert[0].attributes->create_vert_attribute(tri->vert[0], tri->vert[1], tri->vert[2],
                                                       fragment->alpha,
                                                       fragment->vertex_attribute);
    }
}


void run_frag_shader_complete(std::size_t data_begin,std::size_t data_end, void* global_data)
{

}

void rast_tri_execute(std::size_t data_begin,std::size_t data_end, void* global_data)
{
    auto ctx = (Context*)global_data;
    auto& triangle_primitives = ctx->render_passes.back().primitives;
    for (int i = data_begin; i < data_end; ++i)
    {
        auto& triangle_primitive = *triangle_primitives[i];
        if (triangle_primitive.discard || triangle_primitive.inv_cross_dir_z < 0)
        {
            continue;
        }
        rast_tri(triangle_primitive, ctx);
    }
}

void rast_tri_complete(std::size_t data_begin,std::size_t data_end, void* global_data)
{

}

void mid_filter_execute(std::size_t data_begin,std::size_t data_end, void* global_data)
{

}

void mid_filter_complete(std::size_t data_begin,std::size_t data_end, void* global_data)
{
}

void execute_mvp(std::size_t data_begin,std::size_t data_end, void* global_data)
{
    static std::mutex mutex;
    auto* data = (std::tuple<RenderPass*,Mat44>*)global_data;
    Mat44& mvp = std::get<1>(*data);
    auto render_pass = std::get<0>(*data);
    auto get_model_tri = [render_pass](int index, TrianglePrimitive& tri)
    {
        for (int i = 0; i < render_pass->meshes.size(); ++i)
        {
            Mesh& mesh = *Resource::get_resource<Mesh>(render_pass->meshes[i].first);
            if (index < mesh.tri_count())
            {
                mesh.generate_triangle(tri, index);
            }
            index -= mesh.tri_count();
        }
    };


    for (int i = data_begin; i < data_end; ++i)
    {
        render_pass->primitives[i] = render_pass->tri_pool + i;
        auto& triangle_primitive = *render_pass->primitives[i];
        get_model_tri(i, triangle_primitive);
        triangle_primitive.discard = false;
        triangle_primitive.update(mvp);
        // if (triangle_primitives[i].area() > HUGE_TRIANGLE_THR && HUB_TRIANGLE_BREAK_COUNT_MAX > 0)
        // {
        //     std::lock_guard<std::mutex> lock(mutex);
        //     int break_count = HUB_TRIANGLE_BREAK_COUNT_MAX;
        //     std::vector<TrianglePrimitive> result;
        //     break_huge_triangle(triangle_primitives[i], HUGE_TRIANGLE_THR, result, break_count);
        //     triangle_primitives.insert(triangle_primitives.end(), result.begin(), result.end());
        // }
    }
}

void complete_mvp_break_huge_triangle(std::size_t data_begin,std::size_t data_end, void* global_data)
{

}


void vert_view_transform(const L_MATH::Mat<float, 4, 4>& mvp, const int w, const int h,
    VertexAttribute* vertex_attribute, L_MATH::Vec<float, 3>& alpha_, int& i, int& j)
{
    int xy[6];
    Vec3 alpha = vertex_attribute[0].alpha * alpha_[0] + vertex_attribute[1].alpha * alpha_[1] + vertex_attribute[2].
        alpha * alpha_[2];
    Vec3 result;
    auto attributes = vertex_attribute->attributes;
    //每个三角形的三个顶点的vertex_attribute.v是一致的
    for (int k = 0; k < 3; ++k)
    {
        attributes->get_attribute_value(vertex_attribute->v[k], 0, result);
        auto var = (Vec4)(mvp * Vec4(result, 1));
        var /= var[3];
        view_transform(w, h, var[0], var[1], xy[k * 2], xy[k * 2 + 1]);
    }
    i = static_cast<int>(xy[0] * alpha[0] + xy[2] * alpha[1] + xy[4] * alpha[2]);
    j = static_cast<int>(xy[1] * alpha[0] + xy[3] * alpha[1] + xy[5] * alpha[2]);
}

bool add_fragment(TrianglePrimitive& tri, Context* ctx,L_MATH::Vec<float, 3>& alpha, int i, int j)
{
    auto w = ctx->window_handle->w;
    int index = i * w + j;
    auto st_z = tri.v[0][2] * alpha[0] + tri.v[1][2] * alpha[1] + tri.v[2][2] * alpha[2];
    auto current_render_pass = ctx->current_render_pass();
    auto& pass_node = current_render_pass->pass_node;
    auto depth = pass_node.camera->depth_buff[index];
    if (depth < st_z)
    {
        return false;
    }
    auto& fragment = pass_node.camera->fragment_map[index];
    if (L_MATH::is_zero(depth - st_z) && fragment.triangle && fragment.triangle->id < tri.id)
    {
        return false;
    }
    auto alpha_inv_w = 1 / L_MATH::dot(alpha, tri.inv_w);
    pass_node.camera->depth_buff[index] = st_z;
    alpha *= tri.inv_w * alpha_inv_w;
    auto real_z = -1 * alpha_inv_w;
    fragment.triangle = &tri;
    fragment.frag_coord = Vec3{(float)j, (float)i, real_z};
    fragment.alpha = alpha;
    fragment.resolution[0] = w;
    fragment.pass = current_render_pass;
    fragment.resolution[1] = ctx->window_handle->h;
    return true;
}

bool ray_caster(Context* ctx, float si, float sj, RayCasterResult* result)
{
    // TIME_RUN_BEGIN()

    auto current_render_pass = ctx->current_render_pass();
    auto& pass_node = current_render_pass->pass_node;
    Vec3 sv = Vec3({si, sj, -1});
    float min_z = -pass_node.camera->far;
    bool _is_ray_caster = false;
    auto& proj_triangle_list = current_render_pass->primitives;
    L_MATH::Vec<float, 3> alpha;
    for (int i = 0; i < proj_triangle_list.size(); ++i)
    {
        auto& triangle = *proj_triangle_list[i];
        if (triangle.discard)
        {
            continue;
        }
        if (triangle.inv_cross_dir_z < 0)
        {
            continue;
        }
        if (!triangle.box.inside<2>(sv))
        {
            continue;
        }
        triangle.barycentric(sv, alpha);
        if (triangle.is_same_sign(alpha))
        {
            auto alpha_inv = 1 / L_MATH::dot(alpha, triangle.inv_w);
            auto z = -1.f * alpha_inv;
            alpha = alpha * triangle.inv_w * alpha_inv;
            if (z < -pass_node.camera->near && z > min_z)
            {
                result->triangle = &triangle;
                result->alpha = alpha;
                result->z = z;
                min_z = z;
                _is_ray_caster = true;
            }
        }
    }
    return _is_ray_caster;
}

bool ray_caster_bvh(Camera* camera,BVHTree* bvh_tree,float si, float sj, RayCasterResult* result)
{
    Vec3 sv = Vec3({si, sj, -1});
    float min_z = -camera->far;
    bool _is_ray_caster = false;
    std::vector<RayCasterResult> ray_casters;
    if (bvh_tree->intersect_traverse(sv, L_MATH::FORWARD, ray_casters))
    {
        for (auto ray_caster : ray_casters)
        {
            auto& triangle = *ray_caster.triangle;
            if (triangle.discard)
            {
                continue;
            }
            auto alpha = ray_caster.alpha;
            auto alpha_inv = 1 / L_MATH::dot(alpha, triangle.inv_w);
            auto z = -alpha_inv;
            alpha = alpha * triangle.inv_w * alpha_inv;
            if (z > min_z && z < -camera->near)
            {
                result->triangle = &triangle;
                result->alpha = alpha;
                result->z = z;
                min_z = result->z;
                _is_ray_caster = true;
            }
        }
    }
    return _is_ray_caster;
}

bool ray_caster_bvh_priqueue(Camera* camera, BVHTree* bvh_tree, float si, float sj, RayCasterResult* result)
{
    Vec3 sv = Vec3({si, sj, -1});
    float min_z = -camera->far;
    bool _is_ray_caster = false;
    auto box_distance = [](Box3D& box)
    {
        if (box.max[2] < 1 && box.min[2] > -1)
        {
            return -box.min[2];
        }
        return -INFINITY;
    };
    auto tri_distance = [](RayCasterResult* result)
    {
        auto& alpha = result->alpha;
        auto triangle_primitive = result->triangle;
        if (triangle_primitive->discard)
        {
            return -INFINITY;
        }
        return -(triangle_primitive->v[0][2] * alpha[0]
            + triangle_primitive->v[1][2] * alpha[1]
            + triangle_primitive->v[2][2]
            * alpha[2]);
    };

    if (bvh_tree->intersect_compare_distance(sv, L_MATH::FORWARD, result, box_distance, tri_distance))
    {
        auto& triangle = *result->triangle;
        auto alpha = result->alpha;
        auto alpha_inv = 1 / L_MATH::dot(alpha, triangle.inv_w);
        auto z = -alpha_inv;
        alpha = alpha * triangle.inv_w * alpha_inv;
        if (z > min_z && z < -camera->near)
        {
            result->triangle = &triangle;
            result->alpha = alpha;
            result->z = z;
            min_z = result->z;
            _is_ray_caster = true;
        }
    }
    return _is_ray_caster;
}

void rast_tri(TrianglePrimitive& tri, Context* ctx)
{
    auto h = ctx->window_handle->h;
    auto w = ctx->window_handle->w;
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
        if (HUB_TRIANGLE_BREAK_COUNT_MAX == 0)
        {
            view_transform(w, h, clip_vertices[i][0], clip_vertices[i][1], clip_vertices_int[i][0],
                           clip_vertices_int[i][1]);
        }
        else
        {
            // vert_view_transform(ctx->main_camera->view_mat, w, h, tri.vert, alpha_array[i], clip_vertices_int[i][0],
            //                     clip_vertices_int[i][1]);
        }


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
            for (int i = l0x1 + step; i != l0x2 + step; i += step)
            {
                t = (i - l0x1) * 1.0 / (l0x2 - l0x1);
                alpha = alpha_l0 * (1 - t) + l_alpha1 * t;
                add_fragment(tri, ctx, alpha, l0y, i);
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

            add_fragment(tri, ctx, l_alpha, ly, lx);

        }

        if (next_right)
        {
            r_bresenham_line.next_point(rx, ry, rmx, rmy);
            // float t = (ry - r_bresenham_line.y1) * 1.0f / (r_bresenham_line.dy);
            // r_alpha = alpha_array[(r_line + 1) % vert_count] * (1 - t) + alpha_array[r_line] * t;

            invert_view_transform(w, h, rx, ry, st_r[0], st_r[1]);
            tri.barycentric(st_r, r_alpha);
            add_fragment(tri, ctx, r_alpha, ry, rx);

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
                add_fragment(tri, ctx, alpha, ry, i);
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









