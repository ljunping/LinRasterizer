//
// Created by Lin on 2024/11/20.
//
#include "RasterizerJob.h"

#include <random>
#include <__random/random_device.h>

#include "BresenhamLine.h"
#include "BVHTree.h"
#include "Camera.h"
#include "PODPool.h"
#include "FragShader.h"
#include "WindowHandle.h"
#include "TrianglePrimitive.h"
#include "Context.h"
#include "DrawCallContext.h"
#include "Mesh.h"
#include "Transform.h"
#include "VertShader.h"

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
    auto& draw_call_context_tuple = *(std::tuple<DrawCallContext*, int>*)global_data;

    DrawCallContext* draw_call = std::get<0>(draw_call_context_tuple);
    int msaa_index = std::get<1>(draw_call_context_tuple);
    int w = draw_call->w, h = draw_call->h;
    RayCasterResult result;
    auto& view_frustum = draw_call->view_frustum;
    auto near = view_frustum.near;
    float half_height = tan(L_MATH::deg2rad(view_frustum.fov / 2)) * near;
    float half_width = half_height * view_frustum.aspect_ratio;
    for (int idx = data_begin; idx < data_end; idx++)
    {
        int i = idx / w, j = idx % w;
        float y, x;
        invert_view_transform(draw_call, msaa_index, w, h, j, i, x, y);
        Vec3 pos(half_width * x, half_height * y, -near);
        if (draw_call->setting.build_bvh)
        {
            if (!ray_caster_bvh(draw_call, draw_call->bvh_tree, pos, pos.normalize(), result))
            {
                continue;
            }
        }else
        {
            if (!ray_caster(draw_call, pos, pos.normalize(), result))
            {
                continue;
            }
        }
        add_fragment(*result.triangle, draw_call, msaa_index, result.alpha, i, j);
    }
}

void global_path_ray_cast_execute(std::size_t data_begin, std::size_t data_end, void* global_data)
{
    DrawCallContext* ctx = (DrawCallContext*)global_data;
    int w = ctx->w, h = ctx->h;
    auto global_path_trace_samples = ctx->setting.global_path_trace_samples;
    auto& view_frustum = ctx->view_frustum;
    auto near = view_frustum.near;
    float half_height = tan(L_MATH::deg2rad(view_frustum.fov / 2)) * near;
    float half_width = half_height * view_frustum.aspect_ratio;
    auto camera_mat = ctx->inv_view_matrix;
    auto& camera_pos = static_cast<Vec3&>(camera_mat[3]);
    auto frame_buff = ctx->frame_buff;
    auto& lights = ctx->emit_render_node;
    for (int idx = data_begin; idx < data_end; idx++)
    {
        int j = idx / w, i = idx % w;
        float delta = 1.f / (global_path_trace_samples + 1);
        Vec3 radiance;
        int count = 0;
        for (int _k = 0; _k < global_path_trace_samples; ++_k)
        {
            for (int _m = 0; _m < global_path_trace_samples; ++_m)
            {
                Vec3 ray_pos(2 * half_width * (i + delta * _k - 0.5f) / (w - 1) - half_width,
                             2 * half_height * (j + delta * _m - 0.5f) / (h - 1) - half_height, -near);
                L_MATH::pos3_l_dot_mat44(ray_pos, camera_mat);
                Vec3 dir = (ray_pos - camera_pos).normalize();
                Ray ray{ray_pos, dir};
                auto ray_shader = calculate_ray_shader(ctx, lights, ray, ctx->setting.global_ray_trace_max_depth);
                radiance += ray_shader;
                ++count;
            }
        }
        ++global_count;
        if (global_count * 100 % (w * h) == 0)
        {
            printf("global_path_ray_cast_execute percent %.2f\n", global_count * 1.0 / (w * h));
        }
        radiance /= count;
        L_MATH::clamp(radiance,Vec3::ZERO,Vec3::ONE);
        frame_buff[(h - j - 1) * w + i] = l_color(radiance);
    }
}

void ray_cast_frag_complete(std::size_t data_begin,std::size_t data_end, void* global_data)
{
    auto draw_call_context_tuple = (std::tuple<DrawCallContext*, int>*)global_data;
    delete draw_call_context_tuple;
}

void mid_filter_execute(std::size_t data_begin,std::size_t data_end, void* global_data)
{
    DrawCallContext* ctx = (DrawCallContext*)global_data;
    int w = ctx->w, h = ctx->h;
    auto frame_buff = ctx->frame_buff;
    auto global_mid_filter_size = ctx->setting.global_mid_filter_size;
    std::vector<Color> colors(global_mid_filter_size * global_mid_filter_size);
    for (int idx = data_begin; idx < data_end; idx++)
    {
        int y = idx / w, x = idx % w;
        int size = 0;
        Vec3 color;
        for (int i = 0; i < global_mid_filter_size; ++i)
        {
            for (int j = 0; j < global_mid_filter_size; ++j)
            {
                if (y + j >= h || (x + i) >= w)
                {
                    continue;
                }
                size++;
                color += v_color(frame_buff[(y + j) * w + x + i]);
            }
        }
        frame_buff[idx] = l_color(color / size);
    }
}





L_MATH::Vec<float, 3> calculate_ray_shader(DrawCallContext* ctx, std::vector<int>& lights, Ray& ray_in,
                                           int depth)
{
    if (depth == 0)
    {
        return Vec3::ZERO;
    }
    RayCasterResult min_RayCasterResult;
    if (!ctx->bvh_tree->intersect_nearest(ray_in.origin, ray_in.direction, min_RayCasterResult, nullptr, nullptr))
    {
        return Vec3::ZERO;
    }
    std::vector<SHARE_PTR<Pdf>> pdfs;
    Vec3 hit_pos = ray_in.get_point(min_RayCasterResult.t);
    for (auto& light_index : lights)
    {
        auto& light = ctx->nodes[light_index];
        auto light_comp = light.transform->get_component<Light>();
        if (light_comp != nullptr)
        {
            pdfs.emplace_back(std::make_shared<HittablePdf>(*light.transform_geometry, hit_pos));
        }
    }
    auto& geometry_render_node = *ctx->geometry_2_node[min_RayCasterResult.geometry->id];
    auto& material = geometry_render_node.materials[0];
    auto& texture = geometry_render_node.textures[0];

    auto hit_uv = min_RayCasterResult.geometry->get_uv(hit_pos);
    Vec3 texture_color;
    texture->sample(hit_uv, 0, (static_cast<Vec4&>(texture_color)));
    Vec3 emit_color = texture_color * material->emit(ray_in, min_RayCasterResult);
    ScatterResult scatter_result{};
    if (!material->scatter(ray_in, min_RayCasterResult, scatter_result))
    {
        return emit_color;
    }
    if (scatter_result.skip_pdf)
    {
        return texture_color * calculate_ray_shader(ctx, lights, scatter_result.skip_pdf_ray, depth - 1);
    }
    pdfs.emplace_back(scatter_result.pdf);
    auto mix_pdf = std::make_shared<MixPdf>(pdfs);
    auto dir = mix_pdf->generate();
    auto sample_pdf = mix_pdf->value(dir);
    Ray scattered_ray{hit_pos, dir};
    auto scatter_pdf = material->scatter_pdf(scattered_ray, min_RayCasterResult, scatter_result);
    auto color_from_scatter = texture_color * scatter_pdf * calculate_ray_shader(ctx, lights, scattered_ray, depth - 1) / sample_pdf;
    return emit_color + color_from_scatter;

}


void clear_frame_buff_execute(std::size_t data_begin, std::size_t data_end, void* global_data)
{
    DrawCallContext* ctx = (DrawCallContext*)global_data;
    auto frame_buff = ctx->frame_buff;
    auto background_color = ctx->setting.background_color;
    for (int idx = data_begin; idx < data_end; idx++)
    {
        frame_buff[idx] = background_color;
    }
}
void clear_depth_execute(std::size_t data_begin, std::size_t data_end, void* global_data)
{
    auto ctx = (DrawCallContext*)global_data;
    int w = ctx->w;
    int h = ctx->h;
    for (int idx = data_begin; idx < data_end; idx++)
    {
        for (int i = 0; i < ctx->setting.msaa_factor; ++i)
        {
            ctx->depth_buff[idx + i * w * h] = 1.f;
        }
    }
}

void clear_fragment_execute(std::size_t data_begin,std::size_t data_end, void* global_data)
{
    auto ctx = (DrawCallContext*)global_data;
    int w = ctx->w;
    int h = ctx->h;
    for (int idx = data_begin; idx < data_end; idx++)
    {
        for (int i = 0; i < ctx->setting.msaa_factor; ++i)
        {
            auto& fragment = ctx->fragment_map[idx + i * w * h];
            ctx->depth_buff[idx + i * w * h] = 1.f;
            fragment.triangle = nullptr;
            fragment.draw_call = nullptr;
            if (!fragment.build_interpolation_data)
            {
                new(&fragment.interpolation_data) VertexInterpolation();
                fragment.build_interpolation_data = true;
            }
        }
    }
}

void clear_camera_complete(std::size_t data_begin,std::size_t data_end, void* global_data)
{

}

void run_frag_shader_execute(std::size_t data_begin,std::size_t data_end, void* global_data)
{
    DrawCallContext* draw_call = (DrawCallContext*)global_data;
    int w = draw_call->w, h = draw_call->h;
    auto frag_shader = draw_call->frag_shader;
    Color* frame_buff = draw_call->frame_buff;
    auto msaa_factor = draw_call->setting.msaa_factor;
    auto enable_edge = draw_call->setting.enable_edge;
    auto fragment_map = draw_call->fragment_map;
    for (int idx = data_begin; idx < data_end; ++idx)
    {
        Vec4 frag_color;
        Vec4 sample_frag_color;
        int i = idx / w, j = idx % w;
        auto buff_index = (h - i - 1) * w + j;
        Vec4 dts_color = v_color4(frame_buff[buff_index]);
        int sample_count = 0;
        bool is_run_frag = false;
        for (int f = 0; f < msaa_factor; ++f)
        {
            auto _frag_index = idx + f * w * h;
            auto& fragment = (fragment_map)[_frag_index];
            if (fragment.draw_call != draw_call)
            {
                continue;
            }
            auto tri = fragment.triangle;
            if (tri)
            {
                sample_count++;
                if (enable_edge)
                {
                    continue;
                }
                if (!is_run_frag)
                {
                    is_run_frag = true;
                    sample_frag_color = frag_shader->run(_frag_index);
                }
                frag_color += sample_frag_color;
            }
        }
        if (sample_count <= 0)
            continue;
        if (!draw_call->setting.enable_edge)
        {
            frag_color /= msaa_factor;
            frame_buff[buff_index] = blend4(frag_color, dts_color, frag_color[3]);
        }
        else
        {
            if (sample_count < msaa_factor)
            {
                frame_buff[buff_index] = draw_call->setting.edge_color;
            }
        }
    }
}

void interpolation_frag_output_execute(std::size_t data_begin,std::size_t data_end, void* global_data)
{
    DrawCallContext* draw_call = (DrawCallContext*)global_data;
    int w = draw_call->w, h = draw_call->h;
    for (int idx = data_begin; idx < data_end; ++idx)
    {
        for (int i = 0; i < draw_call->setting.msaa_factor; ++i)
        {
            auto fragment = &draw_call->fragment_map[idx + i * w * h];
            auto tri = fragment->triangle;
            if (!tri || (fragment->draw_call != draw_call))
            {
                continue;
            }
            draw_call->create_vert_attribute(tri->gl_vert_index[0], tri->gl_vert_index[1], tri->gl_vert_index[2],
                                             fragment->alpha,
                                             fragment->interpolation_data);

        }
    }
}

void clear_vert_output_execute(std::size_t data_begin, std::size_t data_end, void* global_data)
{
    DrawCallContext* draw_call = (DrawCallContext*)global_data;
    for (int idx = data_begin; idx < data_end; ++idx)
    {
        if (draw_call->vert_invalid[idx])
        {
            draw_call->gl_verts[idx].vertex_interpolation.output.tbn_light_dirs.~vector();
            draw_call->gl_verts[idx].vertex_interpolation.output.view_light_dirs.~vector();
        }
    }
}


void run_frag_shader_complete(std::size_t data_begin,std::size_t data_end, void* global_data)
{
    DrawCallContext* draw_call = (DrawCallContext*)global_data;
    draw_call->frag_shader->end_draw_call(draw_call);
}

void run_vert_shader_execute(std::size_t data_begin, std::size_t data_end, void* global_data)
{
    auto draw_call_context = (DrawCallContext*)global_data;
    int mesh_vert_begin_index = 0;
    int begin_mesh = draw_call_context->get_mesh_index(data_begin, mesh_vert_begin_index);
    SHARE_PTR<Mesh> mesh = draw_call_context->meshes[begin_mesh];
    for (int i = data_begin; i < data_end; ++i)
    {
        draw_call_context->create_vert(i, mesh_vert_begin_index, mesh);
        mesh_vert_begin_index++;
        if (mesh->vert_count<=mesh_vert_begin_index)
        {
            begin_mesh++;
            if (begin_mesh >= draw_call_context->meshes.size())
            {
                break;
            }
            mesh = draw_call_context->meshes[begin_mesh];
            mesh_vert_begin_index = 0;
        }
    }
}

void rast_tri_execute(std::size_t data_begin,std::size_t data_end, void* global_data)
{
    auto& draw_call_context_tuple = *static_cast<std::tuple<DrawCallContext*, int>*>(global_data);
    auto draw_call_context = std::get<0>(draw_call_context_tuple);
    auto msaa_index = std::get<1>(draw_call_context_tuple);
    auto tri_pool = dynamic_cast<TrianglePrimitive*>(draw_call_context->geometrie_pools[TRI]);
    for (int i = data_begin; i < data_end; ++i)
    {
        if (!draw_call_context->geometry_invalid[TRI][i])
        {
            continue;
        }
        auto& triangle_primitive = *(tri_pool + i);
        if (triangle_primitive.out_near
            || triangle_primitive.cache_normal_dir[2] < 0
            || triangle_primitive.
            cache_inv_cross_dir_z < 0)
        {
            continue;
        }
        rast_tri(triangle_primitive, draw_call_context,msaa_index);
    }
}

void rast_tri_complete(std::size_t data_begin,std::size_t data_end, void* global_data)
{
    auto draw_call_context_tuple = (std::tuple<DrawCallContext*, int>*)global_data;
    delete draw_call_context_tuple;
}



void mid_filter_complete(std::size_t data_begin,std::size_t data_end, void* global_data)
{
}


void find_clip_vert(std::vector<int>& keep_gl_vert_indes, L_MATH::Vec<float, 3>& clip_alpha,
                    int& clip_vert_index)
{
    if (clip_alpha[0] > 0 && std::ranges::find(keep_gl_vert_indes, 0) ==
        keep_gl_vert_indes.end())
    {
        clip_vert_index = 0;
    }
    if (clip_alpha[1] > 0 && std::ranges::find(keep_gl_vert_indes, 1) ==
        keep_gl_vert_indes.end())
    {
        clip_vert_index = 1;
    }
    if (clip_alpha[2] > 0 && std::ranges::find(keep_gl_vert_indes, 2) ==
        keep_gl_vert_indes.end())
    {
        clip_vert_index = 2;
    }
}

void process_rast_tri(DrawCallContext* draw_call, int mesh_tri_index, int mesh_index, int idx, TrianglePrimitive* tri)
{

    std::vector<Vec3> clip_alphas;
    if (draw_call->get_clip_near_plane_alpha(tri, clip_alphas))
    {
        return;
    }
    tri->out_near = true;
    if (clip_alphas.empty())
    {
        return;
    }
    std::vector<int> gl_vert_indes;
    std::vector<int> keep_gl_vert_indes;
    auto mesh_vert_count = +draw_call->get_mesh_vert_count();
    for (int j = 0; j < clip_alphas.size(); ++j)
    {
        auto& clip_alpha = clip_alphas[j];
        auto find_var = clip_alpha.find_index(1);
        if (find_var != -1)
        {
            keep_gl_vert_indes.emplace_back(find_var);
        }
    }
    int _index = 0;
    for (auto & clip_alpha : clip_alphas)
    {
        auto find_var = clip_alpha.find_index(1);
        if (find_var != -1)
        {
            gl_vert_indes.emplace_back(tri->gl_vert_index[find_var]);
        }
        else
        {
            int _new_index = mesh_vert_count + 2 * idx + _index;
            gl_vert_indes.emplace_back(_new_index);
            if (!draw_call->vert_invalid[_new_index] || draw_call->gl_verts[_new_index].mesh_vert)
            {
                draw_call->create_vert(_new_index, mesh_tri_index,
                                   draw_call->meshes[mesh_index], clip_alpha);
            }
            _index++;
        }
    }
    draw_call->create_tri(idx, gl_vert_indes[0], gl_vert_indes[1], gl_vert_indes[2]);
    if (gl_vert_indes.size()==4)
    {
        RUNTIME_ASSERT(gl_vert_indes.size()==4, "gl_vert_indes.size()!=4");
        draw_call->create_tri(idx + draw_call->get_mesh_tri_count(), gl_vert_indes[2], gl_vert_indes[3],
                              gl_vert_indes[0]);
    }
}

void run_process_tri_primitive(std::size_t data_begin,std::size_t data_end, void* global_data)
{
    auto* draw_call = static_cast<DrawCallContext*>(global_data);
    int mesh_tri_index;
    int mesh_index = draw_call->get_mesh_index_by_tri_index(data_begin, mesh_tri_index);
    auto mesh = draw_call->meshes[mesh_index];
    for (int idx = data_begin; idx < data_end; ++idx)
    {
        auto tri = draw_call->create_tri(idx, mesh_index, mesh_tri_index);
        if (!draw_call->setting.enable_ray_cast)
        {
            process_rast_tri(draw_call, mesh_tri_index, mesh_index, idx, tri);
        }
        mesh_tri_index++;
        if (mesh->tri_count() != mesh_tri_index)
            continue;
        mesh_index++;
        if (mesh_index >= draw_call->meshes.size())
        {
            break;
        }
        mesh = draw_call->meshes[mesh_index];
        mesh_tri_index = 0;
    }
}

void prepare_ctx_execute(std::size_t data_begin, std::size_t data_end, void* global_data)
{
    auto* draw_call = static_cast<DrawCallContext*>(global_data);
    for (int idx = data_begin; idx < data_end; ++idx)
    {
        if (idx < draw_call->gl_vert_count)
        {
            draw_call->vert_invalid[idx] = false;
        }
        if (idx < draw_call->geometry_count[TRI])
        {
            draw_call->geometry_invalid[TRI][idx] = false;
        }
    }

}


void view_transform(DrawCallContext* draw_call_context, int msaa_index, int w, int h, float st_x, float st_y, int& i,
                    int& j)
{
    Vec2 offset;
    if (draw_call_context->setting.enable_edge)
    {
        offset = edge_msaa_template(draw_call_context->setting.msaa_factor, msaa_index);
    }
    else
    {
        offset = msaa_template(draw_call_context->setting.msaa_factor, msaa_index);
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


void invert_view_transform(DrawCallContext* call_context,int msaa_index,int w, int h, float i, float j, float& st_x, float& st_y)
{
    Vec2 offset;
    if(call_context->setting.enable_edge)
    {
        offset = edge_msaa_template(call_context->setting.msaa_factor, msaa_index);
    }else
    {
        offset = msaa_template(call_context->setting.msaa_factor, msaa_index);
    }
    st_x = (i * 2 - (w - 1)) * 1.0 / (w - 1) + offset[0] * 2 / (w - 1);
    st_y = (j * 2 - (h - 1)) * 1.0 / (h - 1) + offset[1] * 2 / (h - 1);
}
//
// void vert_view_transform(DrawCallContext* draw_call_context,const L_MATH::Mat<float, 4, 4>& mvp, const int w, const int h,
//                          VertexInterpolation* vertex_attribute, L_MATH::Vec<float, 3>& alpha_, int& i, int& j)
// {
//     int xy[6];
//     Vec3 alpha = vertex_attribute[0].alpha * alpha_[0] + vertex_attribute[1].alpha * alpha_[1] + vertex_attribute[2].
//         alpha * alpha_[2];
//     Vec3 result;
//     Vec4& result4 = static_cast<L_MATH::Vec<float, 4>&>(result);
//     result4[3] = 1;
//     auto attributes = vertex_attribute->mesh_ptr;
//     //每个三角形的三个顶点的vertex_attribute.v是一致的
//     for (int k = 0; k < 3; ++k)
//     {
//         attributes->get_attribute_value(vertex_attribute->mesh_vert_index[k], 0, result);
//         result4 = result4 * mvp;
//         result4 /= result4[3];
//         view_transform(draw_call_context,w, h, result4[0], result4[1], xy[k * 2], xy[k * 2 + 1]);
//     }
//     i = static_cast<int>(xy[0] * alpha[0] + xy[2] * alpha[1] + xy[4] * alpha[2]);
//     j = static_cast<int>(xy[1] * alpha[0] + xy[3] * alpha[1] + xy[5] * alpha[2]);
// }


bool add_fragment(TrianglePrimitive& tri, DrawCallContext* draw_call,int msaa_index,const L_MATH::Vec<float, 3>& alpha, int i, int j)
{
    int w = draw_call->w, h = draw_call->h;
    auto _index = i * w + j;
    if (_index < 0 || _index >= w * h)
    {
        return false;
    }
    int index = _index + w * h * msaa_index;
    auto& fragment = draw_call->fragment_map[index];
    auto st_z = tri.v[0][2] * alpha[0] + tri.v[1][2] * alpha[1] + tri.v[2][2] * alpha[2];
    auto real_z = st_z;
    if (draw_call->setting.enable_ray_cast)
    {
        auto& view_frustum = draw_call->view_frustum;
        st_z = 2 * (st_z + view_frustum.near) / (view_frustum.near - view_frustum.far) - 1;
    }
    if (draw_call->setting.enable_depth)
    {
        auto depth = draw_call->depth_buff[index];
        if (depth < st_z || (L_MATH::is_zero(depth - st_z)))
        {
            return false;
        }
        if (draw_call->setting.enable_depth_write)
        {
            draw_call->depth_buff[index] = st_z;
        }
    }
    if (draw_call->setting.enable_ray_cast)
    {
        fragment.triangle = &tri;
        fragment.frag_coord = Vec3{(float)j, (float)i, real_z,};
        fragment.alpha = alpha;
        fragment.draw_call = draw_call;
    }
    else
    {
        auto alpha_inv_w = 1 / L_MATH::dot(alpha, tri.cache_inv_w);
        real_z = -1 * alpha_inv_w;
        fragment.triangle = &tri;
        fragment.frag_coord = Vec3{(float)j, (float)i, real_z,};
        fragment.alpha = alpha * tri.cache_inv_w * alpha_inv_w;
        fragment.draw_call = draw_call;
    }
    return true;
}

bool ray_caster(DrawCallContext* draw_call,const Vec3& origin,const Vec3& dir, RayCasterResult& result)
{
    auto _max_z = -draw_call->view_frustum.near;
    float min_z = -draw_call->view_frustum.far;

    bool _is_ray_caster = false;
    auto proj_triangle_list = dynamic_cast<TrianglePrimitive*>(draw_call->geometrie_pools[TRI]);
    RayCasterResult _result;
    for (int i = 0; i < draw_call->geometry_count[TRI]; ++i)
    {
        if (!draw_call->geometry_invalid[TRI][i])
        {
            continue;
        }
        auto& triangle = *(proj_triangle_list + i);
        if (triangle.intersect(origin, dir, &_result))
        {
            if (_result.t < 0)
            {
                continue;
            }
            float w_z = origin[2] + dir[2] * _result.t;
            if (w_z > min_z && w_z < _max_z)
            {
                min_z = w_z;
                result = _result;
                _is_ray_caster = true;
            }
        }
    }
    return _is_ray_caster;
}

bool ray_caster_bvh(DrawCallContext* ctx,BVHTree* bvh_tree,const Vec3& origin,const Vec3& dir, RayCasterResult& result)
{
    bool _is_ray_caster = false;
    auto _max_z = -ctx->view_frustum.near;
    auto _min_z = -ctx->view_frustum.far;
    std::vector<RayCasterResult> ray_casters;
    if (bvh_tree->intersect_traverse(origin, dir, ray_casters, nullptr, nullptr))
    {
        for (auto& ray_caster : ray_casters)
        {
            if (!ray_caster.triangle)
            {
                continue;
            }
            auto alpha = ray_caster.alpha;
            float view_z = origin[2] + dir[2] * ray_caster.t;
            if (view_z > _min_z && view_z < _max_z)
            {
                result.geometry = ray_caster.geometry;
                result.triangle = ray_caster.triangle;
                result.alpha = alpha;
                _min_z = view_z;
                _is_ray_caster = true;
            }
        }
    }
    return _is_ray_caster;
}


// bool ray_caster_bvh_priqueue(DrawCallContext* ctx, BVHTree* bvh_tree, float si, float sj, RayCasterResult* result)
// {
//     Vec3 sv = Vec3({si, sj, -1});
//     bool _is_ray_caster = false;
//     auto box_distance = [](Box<3>& box)
//     {
//         if (box.max[2] < 1 && box.min[2] > -1)
//         {
//             return -box.min[2];
//         }
//         return -INFINITY;
//     };
//     auto tri_distance = [](RayCasterResult* result)
//     {
//         auto& alpha = result->alpha;
//         auto triangle_primitive = result->triangle;
//         if (triangle_primitive->out_near || !triangle_primitive->ccw())
//         {
//             return -INFINITY;
//         }
//         return -(triangle_primitive->v[0][2] * alpha[0]
//             + triangle_primitive->v[1][2] * alpha[1]
//             + triangle_primitive->v[2][2]
//             * alpha[2]);
//     };
//
//     if (bvh_tree->intersect_compare_distance(sv, L_MATH::FORWARD, result, box_distance, tri_distance))
//     {
//         _is_ray_caster = true;
//     }
//     return _is_ray_caster;
// }
void rast_huge_tri(TrianglePrimitive& tri, Context* ctx)
{

}

void rast_tri(TrianglePrimitive& tri, DrawCallContext* draw_call,int msaa_index)
{
    int w = draw_call->w, h = draw_call->h;
    tri.clip_st_box();
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
            view_transform(draw_call,msaa_index, w, h, clip_vertices[i][0], clip_vertices[i][1], clip_vertices_int[i][0],
                           clip_vertices_int[i][1]);
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
        if ((!l_bresenham_line.has_next() && next_left))
        {

            l_line = (l_line + 1) % vert_count;
            l_alpha = alpha_array[l_line];
            l_bresenham_line = BresenhamLine(clip_vertices_int[l_line][0], clip_vertices_int[l_line][1],
                                             clip_vertices_int[(l_line + 1) % vert_count][0],
                                             clip_vertices_int[(l_line + 1) % vert_count][1]);
            continue;
        }
        if ((!r_bresenham_line.has_next() && next_right))
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
            invert_view_transform(draw_call,msaa_index,w, h, lx, ly, st_l[0], st_l[1]);
            tri.barycentric2D(st_l, l_alpha);
            add_fragment(tri, draw_call,msaa_index, l_alpha, ly, lx);

        }

        if (next_right)
        {
            r_bresenham_line.next_point(rx, ry, rmx, rmy);
            invert_view_transform(draw_call,msaa_index, w, h, rx, ry, st_r[0], st_r[1]);
            tri.barycentric2D(st_r, r_alpha);
            add_fragment(tri, draw_call, msaa_index,r_alpha, ry, rx);
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
                add_fragment(tri, draw_call, msaa_index, l_alpha * (1 - t) + r_alpha * t, ry, i);
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
// void break_huge_triangle(TrianglePrimitive& tri, float area_thr, std::vector<TrianglePrimitive>& result,
//                          int& count)
// {
//     if (tri.area() > area_thr && count > 0)
//     {
//         tri.out_near = true;
//         --count;
//         Vec3 alphas[6];
//         int mesh_vert_index[12] = {0, 1, 5, 1, 2, 3, 1, 3, 5, 5, 3, 4};
//         alphas[0] = Vec3{1, 0, 0};
//         alphas[1] = Vec3{0.5, 0.5, 0};
//         alphas[2] = Vec3{0, 1, 0};
//         alphas[3] = Vec3{0, 0.5, 0.5};
//         alphas[4] = Vec3{0, 0, 1};
//         alphas[5] = Vec3{0.5, 0, 0.5};
//         for (int i = 0; i < 12; i = i + 3)
//         {
//             Vec3 alpha0 = alphas[mesh_vert_index[i]];
//             Vec3 alpha1 = alphas[mesh_vert_index[i + 1]];
//             Vec3 alpha2 = alphas[mesh_vert_index[i + 2]];
//             auto tri_result = Pool<TrianglePrimitive>::POOL.create();
//
//             tri_result->mesh_vert_index[0] = tri.mesh_vert_index[0] * alpha0[0] + tri.mesh_vert_index[1] * alpha0[1] + tri.mesh_vert_index[2] * alpha0[2];
//             tri_result->mesh_vert_index[1] = tri.mesh_vert_index[0] * alpha1[0] + tri.mesh_vert_index[1] * alpha1[1] + tri.mesh_vert_index[2] * alpha1[2];
//             tri_result->mesh_vert_index[2] = tri.mesh_vert_index[0] * alpha2[0] + tri.mesh_vert_index[1] * alpha2[1] + tri.mesh_vert_index[2] * alpha2[2];
//             tri_result->inv_w = Vec3::ONE;
//             tri_result->id = tri.id;
//             auto attributes = tri.vert[0].attributes;
//
//             attributes->create_vert_attribute(tri.vert[0], tri.vert[1], tri.vert[2], alpha0, tri_result->vert[0]);
//
//             attributes->create_vert_attribute(tri.vert[0], tri.vert[1], tri.vert[2], alpha1, tri_result->vert[1]);
//
//             attributes->create_vert_attribute(tri.vert[0], tri.vert[1], tri.vert[2], alpha2, tri_result->vert[2]);
//             tri_result->update_param();
//             break_huge_triangle(*tri_result, area_thr, result, count);
//         }
//     }
//     else
//     {
//         result.emplace_back(tri);
//         Pool<TrianglePrimitive>::POOL.recycle(&tri);
//     }
// }









