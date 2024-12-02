//
// Created by Lin on 2024/11/20.
//

#ifndef RASTERIZERJOB_H
#define RASTERIZERJOB_H
#include "Color.h"
#include "JobSystem.h"
#include "Rasterizer.h"
#include "TrianglePrimitive.h"

#define THREAD_MVP_SIZE 10000
inline float HUGE_TRIANGLE_THR = 0.5;
inline int HUB_TRIANGLE_BREAK_COUNT_MAX = 0;
inline constexpr int CCW[] = {0, 1, 2, 1, 2, 0, 2, 0, 1};

class Rasterizer;
class FragShader;
class RayCasterResult;
enum WORK_TYPE
{
    INVALID,
    RAY_CAST_FRAG,
    CLEAR,
    FRAG_SHADER,
};

struct Work
{
    short block_x;
    short block_y;
    WORK_TYPE TYPE;
};

struct RayCasterWork
{
    Work base;
};

struct ClearWork
{
    Work base;
    Color* buffer;
    Color color;
};
struct FragShaderWork
{
    Work base;
    Color* buffer;
    FragShader* frag_shader;
};

inline void view_transform(const int w, const int h, float st_x, float st_y, int& i,
                           int& j)
{
    i = static_cast<int>((st_x * (w - 1) / 2) + (w - 1) * 1.0 / 2);
    j = static_cast<int>((st_y * (h - 1) / 2) + (h - 1) * 1.0 / 2);
}

inline void invert_view_transform(const int w, const int h, int i, int j, float &st_x, float &st_y)
{
    st_x = (i * 2 - (w - 1))*1.0 / (w - 1);
    st_y = (j * 2 - (h - 1))*1.0 / (h - 1);
}


inline void vert_view_transform(const Mat44& mvp, const int w, const int h, VertexAttribute* vertex_attribute,
                                Vec3& alpha_, int& i,
                                int& j)
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

bool add_fragment(TrianglePrimitive& tri, Rasterizer* rasterizer, L_MATH::Vec<float, 3>& alpha, int i, int j);

bool ray_caster(Camera* camera, float si, float sj, RayCasterResult* result);
bool ray_caster_bvh(Camera* camera, float si, float sj, RayCasterResult* result);
bool ray_caster_bvh_priqueue(Camera* camera, float si, float sj, RayCasterResult* result);

void rast_tri(TrianglePrimitive& tri, Rasterizer* rasterizer);
void break_huge_triangle(TrianglePrimitive& tri, float area_thr, std::vector<TrianglePrimitive>& result, int& count);

void ray_cast_frag_execute(int data_begin, int data_end, void* global_data);
void ray_cast_frag_complete(int data_begin, int data_end, void* global_data);

void clear_execute(int data_begin, int data_end, void* global_data);
void clear_complete(int data_begin, int data_end, void* global_data);

void prepare_frag_shader_execute(int data_begin, int data_end, void *global_data);
void run_frag_shader_execute(int data_begin, int data_end, void* global_data);
void run_frag_shader_complete(int data_begin, int data_end, void* global_data);

void rast_tri_execute(int data_begin, int data_end, void* global_data);
void rast_tri_complete(int data_begin, int data_end, void* global_data);

void mid_filter_execute(int data_begin, int data_end, void* global_data);

void mid_filter_complete(int data_begin, int data_end, void* global_data);

void execute_mvp_break_huge_triangle(int data_begin, int data_end, void* global_data);

void complete_mvp_break_huge_triangle(int data_begin, int data_end, void* global_data);


#endif //RASTERIZERJOB_H
