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

Vec2 MSAA_TEMPLATE_1[1] = {{0, 0}};
Vec2 MSAA_TEMPLATE_2[2]={{-0.25, -0.25},{0.25, 0.25}};
Vec2 MSAA_TEMPLATE_4[4]={{-0.375, -0.125},{0.125, -0.375},{-0.125, 0.375},{0.375, 0.125}};

Vec2 EDGE_MSAA_TEMPLATE_2[2]={{-0.5, -0.5},{0.5, 0.5}};
Vec2 EDGE_MSAA_TEMPLATE_4[4]={{-0.5, -0.5},{0.5, -0.5},{-0.5, 0.5},{0.5, 0.5}};


Vec2 msaa_template(int factor,int index)
{
    if(factor==1) return MSAA_TEMPLATE_1[index];
    if(factor==2) return MSAA_TEMPLATE_2[index];
    if(factor==4) return MSAA_TEMPLATE_4[index];
    throw std::runtime_error("wrong factor");
}


Vec2 edge_msaa_template(int factor,int index)
{
    if(factor==2) return EDGE_MSAA_TEMPLATE_2[index];
    if(factor==4) return EDGE_MSAA_TEMPLATE_4[index];
    throw std::runtime_error("wrong factor");
}

void ray_cast_frag_execute(std::size_t data_begin,std::size_t data_end,  void* global_data)
{
    Context* ctx = (Context*)global_data;
    int w, h;
    ctx->get_screen_size(w, h);
    auto current_render_pass = ctx->current_render_pass();
    RayCasterResult result;
    for (int idx = data_begin; idx < data_end; idx++)
    {
        int i = idx / w, j = idx % w;
        float y,x;
        invert_view_transform(w, h, j, i, x, y);
        if (ctx->setting.build_bvh)
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
    auto background_color = ctx->setting.background_color;
    for (int idx = data_begin; idx < data_end; idx++)
    {
        frame_buff[idx] = background_color;
    }
}


void clear_camera_execute(std::size_t data_begin,std::size_t data_end, void* global_data)
{
    Context* ctx = (Context*)global_data;
    int w, h;
    ctx->get_screen_size(w, h);
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
        for (int i = 0; i < ctx->setting.msaa_factor; ++i)
        {
            pass_node.camera->depth_buff[idx + i * w * h] = 1.f;
            pass_node.camera->fragment_map[idx + i * w * h].triangle = nullptr;
            pass_node.camera->fragment_map[idx + i * w * h].draw_call = nullptr;
        }
    }
}

void clear_complete(std::size_t data_begin,std::size_t data_end, void* global_data)
{

}

void run_frag_shader_execute(std::size_t data_begin,std::size_t data_end, void* global_data)
{

    FragShader* frag_shader = (FragShader*)global_data;
    auto ctx = frag_shader->draw_call->ctx;
    auto draw_call = frag_shader->draw_call;
    int w, h;
    frag_shader->draw_call->ctx->get_screen_size(w, h);
    Color* frame_buff = frag_shader->draw_call->frame_buff;
    for (int idx = data_begin; idx < data_end; ++idx)
    {
        Vec4 frag_color;
        Vec4 sample_frag_color;
        int i = idx / w, j = idx % w;
        Vec4 dts_color = v_color4(frame_buff[(h - i - 1) * w + j]);
        int sample_count = 0;
        bool is_run_frag = false;
        for (int f = 0; f < ctx->setting.msaa_factor; ++f)
        {
            auto& fragment = (*frag_shader->fragment_map)[idx + f * w * h];
            if (fragment.draw_call != draw_call)
            {
                continue;
            }
            auto tri = fragment.triangle;
            if (tri)
            {
                sample_count++;
                if (ctx->setting.enable_edge)
                {
                    continue;
                }
                if (!is_run_frag)
                {
                    is_run_frag = true;
                    sample_frag_color = frag_shader->run(idx + f * w * h);
                }
                frag_color += sample_frag_color;
            }
            else
            {
                frag_color += dts_color;
            }
        }
        if (sample_count <= 0)
            continue;
        if (!ctx->setting.enable_edge)
        {
            frag_color /= ctx->setting.msaa_factor;
            frame_buff[(h - i - 1) * w + j] = blend4(frag_color, dts_color, frag_color[3]);
        }
        else
        {
            if (sample_count < ctx->setting.msaa_factor)
            {
                frame_buff[(h - i - 1) * w + j] = ctx->setting.edge_color;
            }
        }
    }
}

void prepare_frag_shader_execute(std::size_t data_begin,std::size_t data_end, void* global_data)
{
    Context* context = (Context*)global_data;
    int w, h;
    context->get_screen_size(w, h);
    auto current_render_pass = context->current_render_pass();
    auto& pass_node = current_render_pass->pass_node;
    for (int idx = data_begin; idx < data_end; ++idx)
    {

        for (int i = 0; i < context->setting.msaa_factor; ++i)
        {
            auto fragment = &pass_node.camera->fragment_map[idx + i * w * h];
            auto tri = fragment->triangle;
            if (!tri||(fragment->draw_call != current_render_pass))
            {
                continue;
            }
            tri->vert[0].attributes->create_vert_attribute(tri->vert[0], tri->vert[1], tri->vert[2],
                                                           fragment->alpha,
                                                           fragment->vertex_attribute);
        }
    }
}


void run_frag_shader_complete(std::size_t data_begin,std::size_t data_end, void* global_data)
{

}

void rast_tri_execute(std::size_t data_begin,std::size_t data_end, void* global_data)
{
    auto ctx = (Context*)global_data;
    auto current_render_pass = ctx->current_render_pass();
    auto& triangle_primitives = current_render_pass->primitives;
    for (int i = data_begin; i < data_end; ++i)
    {
        auto& triangle_primitive = *triangle_primitives[i];
        if (triangle_primitive.discard || triangle_primitive.inv_cross_dir_z < 0)
        {
            continue;
        }
        for (int j = 0; j < ctx->setting.msaa_factor; ++j)
        {
            rast_tri(triangle_primitive, ctx);
        }
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
    auto* data = (std::tuple<DrawCallData*,Mat44>*)global_data;
    Mat44 &mvp = std::get<1>(*data);
    auto render_pass = std::get<0>(*data);
    auto get_model_tri = [render_pass](int index, TrianglePrimitive& tri)
    {
        for (int i = 0; i < render_pass->meshes.size(); ++i)
        {
            Mesh& mesh = *Resource::get_resource<Mesh>(render_pass->meshes[i].first);
            if (index < mesh.tri_count())
            {
                mesh.generate_triangle(tri, index);
                break;
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

Mesh* generate_sphere(float radius, int stacks, int slices)
{
    float* vert_buff = new float[5 * (stacks + 1) * (slices + 1)];

    // Generate vertices
    for (int stack = 0; stack < stacks; ++stack)
    {
        float phi = M_PI * stack / stacks; // Latitude angle (0 to π)
        for (int slice = 0; slice <= slices; ++slice)
        {
            float theta = 2 * M_PI * slice / slices; // Longitude angle (0 to 2π)
            float x = radius * sin(phi) * cos(theta);
            float y = radius * sin(phi) * sin(theta);
            float z = radius * cos(phi);
            vert_buff[5 * (stack * (slices + 1) + slice)] =x;
            vert_buff[5 * (stack * (slices + 1) + slice) + 1] = y;
            vert_buff[5 * (stack * (slices + 1) + slice) + 2] = z;
            vert_buff[5 * (stack * (slices + 1) + slice) + 3] = stack*1.0f / stacks;
            vert_buff[5 * (stack * (slices + 1) + slice) + 4] = slice * 1.0f / slices;

        }
    }
    SHARE_PTR<float[]> share_vert_buff(vert_buff);
    auto _Attributes = CREATE_OBJECT_BY_TYPE(Mesh, share_vert_buff, 5 * (stacks + 1) * (slices + 1));
    _Attributes->bind_attribute(POS,3, 0, 5);
    _Attributes->bind_attribute(UV,2, 3, 5);
    std::vector<int> eb0;
    // Generate triangles
    for (int stack = 0; stack < stacks; ++stack)
    {
        for (int slice = 0; slice < slices; ++slice)
        {
            // Compute indices for the four vertices of the current quad
            int i1 = stack * (slices + 1) + slice; // Top-left
            int i2 = i1 + 1; // Top-right
            int i3 = i1 + (slices + 1); // Bottom-left
            int i4 = i3 + 1; // Bottom-right
            // Two triangles per quad
            if (stack != 0)
            {
                eb0.emplace_back(i1);
                eb0.emplace_back(i3);
                eb0.emplace_back(i2);
            }
            if (stack != stacks- 1)
            {
                eb0.emplace_back(i3);
                eb0.emplace_back(i4);
                eb0.emplace_back(i2);
            }
        }
    }
    _Attributes->ebo = std::move(eb0);
    return _Attributes;
}



Mesh* generate_quad()
{
    float* vert_buff = new float[4 * 5]
    {
        -0.5, -0.5, 0, 0, 0,
        0.5, -0.5, 0, 1, 0,
        -0.5, 0.5, 0, 0, 1,
        0.5, 0.5, 0, 1, 1,
    };

    SHARE_PTR<float[]> share_vert_buff(vert_buff);
    auto _Attributes = CREATE_OBJECT_BY_TYPE(Mesh, share_vert_buff, 4*5);
    _Attributes->bind_attribute(POS,3, 0, 5);
    _Attributes->bind_attribute(UV, 2, 3, 5);
    _Attributes->ebo = {0, 1, 2, 1, 3, 2};
    return _Attributes;
}

Mesh* generate_tri()
{
    float* vert_buff = new float[3 * 5]
    {
        -0.5, -0.5, 0, 0, 0,
        0.5, -0.5, 0, 1, 0,
        -0.5, 0.5, 0, 0, 1,
    };

    SHARE_PTR<float[]> share_vert_buff(vert_buff);
    auto _Attributes = CREATE_OBJECT_BY_TYPE(Mesh, share_vert_buff, 3*5);
    _Attributes->bind_attribute(POS,3, 0, 5);
    _Attributes->bind_attribute(UV, 2, 3, 5);
    _Attributes->ebo = {0, 1, 2};
    return _Attributes;
}

void draw_line(Context* ctx, Color* buff, void* data)
{
    int w, h;
    ctx->get_screen_size(w, h);
    auto line = (DrawLineInfo*)data;
    auto bresenham_line = BresenhamLine(line->x0, line->y0, line->x1, line->y1);
    int x, y, mx, my;
    x = line->x0;
    y = line->y0;
    buff[y * w + x] = line->color;
    while (bresenham_line.has_next())
    {
        bresenham_line.next_point(x, y, mx, my);
        buff[y * w + x] = line->color;
    }
}

void complete_mvp_break_huge_triangle(std::size_t data_begin,std::size_t data_end, void* global_data)
{

}

void view_transform(int w, int h, float st_x, float st_y, int& i, int& j)
{
    auto ctx = get_current_ctx();
    Vec2 offset;
    if (ctx->setting.enable_edge)
    {
        offset = edge_msaa_template(ctx->setting.msaa_factor, ctx->current_mesa_index());
    }
    else
    {
        offset = msaa_template(ctx->setting.msaa_factor, ctx->current_mesa_index());
    }

    float inv_w_minus_1 = 2.0f / (w - 1);
    float inv_h_minus_1 = 2.0f / (h - 1);

    st_x -= offset[0] * inv_w_minus_1;
    st_y -= offset[1] * inv_h_minus_1;

    float half_w_minus_1 = (w - 1) / 2.0f;
    float half_h_minus_1 = (h - 1) / 2.0f;

    i = std::lround(st_x * half_w_minus_1 + half_w_minus_1);
    j = std::lround(st_y * half_h_minus_1 + half_h_minus_1);
}


void invert_view_transform(int w, int h, int i, int j, float& st_x, float& st_y)
{
    auto ctx = get_current_ctx();
    Vec2 offset;
    if(ctx->setting.enable_edge)
    {
        offset = edge_msaa_template(ctx->setting.msaa_factor, ctx->current_mesa_index());
    }else
    {
        offset = msaa_template(ctx->setting.msaa_factor, ctx->current_mesa_index());
    }
    st_x = (i * 2 - (w - 1)) * 1.0 / (w - 1) + offset[0] * 2 / (w - 1);
    st_y = (j * 2 - (h - 1)) * 1.0 / (h - 1) + offset[1] * 2 / (h - 1);
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

bool add_fragment(TrianglePrimitive& tri, Context* ctx,const L_MATH::Vec<float, 3>& alpha, int i, int j)
{
    int w, h;
    ctx->get_screen_size(w, h);
    auto _index = i * w + j;
    if (_index < 0 || _index >= w * h)
    {
        return false;
    }
    int index = _index + w * h * ctx->current_mesa_index();

    auto current_render_pass = ctx->current_render_pass();
    auto& pass_node = current_render_pass->pass_node;
    auto& fragment = pass_node.camera->fragment_map[index];
    if (ctx->setting.enable_depth)
    {
        auto st_z = tri.v[0][2] * alpha[0] + tri.v[1][2] * alpha[1] + tri.v[2][2] * alpha[2];

        auto depth = pass_node.camera->depth_buff[index];
        if (depth < st_z||(L_MATH::is_zero(depth - st_z)))
        {
            return false;
        }
        if (ctx->setting.enable_depth_write)
        {
            pass_node.camera->depth_buff[index] = st_z;
        }
    }
    auto alpha_inv_w = 1 / L_MATH::dot(alpha, tri.inv_w);
    auto real_z = -1 * alpha_inv_w;
    fragment.triangle = &tri;
    fragment.frag_coord = Vec3{(float)j, (float)i, real_z};
    fragment.alpha =  alpha * tri.inv_w * alpha_inv_w;
    fragment.resolution[0] = w;
    fragment.draw_call = current_render_pass;
    fragment.resolution[1] = h;
    return true;
}

bool ray_caster(Context* ctx, float si, float sj, RayCasterResult* result)
{
    auto current_render_pass = ctx->current_render_pass();
    Vec3 sv = Vec3({si, sj, -1});
    float min_z = 1;
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
            float stz = (triangle.v[0][2] * alpha[0]
                + triangle.v[1][2] * alpha[1]
                + triangle.v[2][2]
                * alpha[2]);

            if (stz < min_z && stz > -1)
            {
                result->triangle = &triangle;
                result->alpha = alpha;
                min_z = stz;
                _is_ray_caster = true;
            }
        }
    }
    return _is_ray_caster;
}

bool ray_caster_bvh(Camera* camera,BVHTree* bvh_tree,float si, float sj, RayCasterResult* result)
{
    Vec3 sv = Vec3({si, sj, -1});
    float min_z = 1;
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
            float stz = (triangle.v[0][2] * alpha[0]
                + triangle.v[1][2] * alpha[1]
                + triangle.v[2][2]
                * alpha[2]);

            if (stz < min_z && stz > -1)
            {
                result->triangle = &triangle;
                result->alpha = alpha;
                min_z = stz;
                _is_ray_caster = true;
            }
        }
    }
    return _is_ray_caster;
}

bool ray_caster_bvh_priqueue(Camera* camera, BVHTree* bvh_tree, float si, float sj, RayCasterResult* result)
{
    Vec3 sv = Vec3({si, sj, -1});
    bool _is_ray_caster = false;
    auto box_distance = [](Box<3>& box)
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
        _is_ray_caster = true;
    }
    return _is_ray_caster;
}
void rast_huge_tri(TrianglePrimitive& tri, Context* ctx)
{

}

void rast_tri(TrianglePrimitive& tri, Context* ctx)
{
    int w,h;
    ctx->get_screen_size(w, h);
    tri.clip();
    auto vert_count = tri.clip_vert_count;
    auto clip_vertices = tri.clip_vertices;
    auto alpha_array = tri.clip_vertices_alpha;
    if (vert_count == 0)
    {
        return;
    }
    auto clip_vertices_int = new int[vert_count][2];
    for (int i = 0; i < vert_count; ++i)
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
    int l_line = 0;
    int l_y = clip_vertices_int[0][1],
        l_x = clip_vertices_int[0][0];


    for (int i = 0; i < vert_count; ++i)
    {
        if (clip_vertices_int[i][1] < l_y||(clip_vertices_int[i][1] == l_y && clip_vertices_int[i][0] < l_x))
        {
            l_y = clip_vertices_int[i][1];
            l_x = clip_vertices_int[i][0];
            l_line = i;
        }
    }
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
        if (jump_line_l || (!l_bresenham_line.has_next() && next_left))
        {
            l_line = (l_line + 1) % vert_count;
            l_alpha = alpha_array[l_line];
            l_bresenham_line = BresenhamLine(clip_vertices_int[l_line][0], clip_vertices_int[l_line][1],
                                             clip_vertices_int[(l_line + 1) % vert_count][0],
                                             clip_vertices_int[(l_line + 1) % vert_count][1]);
            continue;
        }
        if (jump_line_r || (!r_bresenham_line.has_next() && next_right))
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
            invert_view_transform(w, h, lx, ly, st_l[0], st_l[1]);
            tri.barycentric(st_l, l_alpha);
            add_fragment(tri, ctx, l_alpha, ly, lx);

        }

        if (next_right)
        {
            r_bresenham_line.next_point(rx, ry, rmx, rmy);
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
            for (int i = lx + step; i != rx; i += step)
            {
                t = (i - lx) * 1.0 / (rx - lx);
                add_fragment(tri, ctx, l_alpha * (1 - t) + r_alpha * t, ry, i);
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
            tri_result->id = tri.id;
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









