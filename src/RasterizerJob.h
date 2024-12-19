//
// Created by Lin on 2024/11/20.
//

#ifndef RASTERIZERJOB_H
#define RASTERIZERJOB_H
#include "Color.h"
#include <vector>

class BVHTree;
class VertexAttribute;
struct RayCasterResult;
class Camera;
class Context;
class TrianglePrimitive;
inline float HUGE_TRIANGLE_THR = 0.5;
inline int HUB_TRIANGLE_BREAK_COUNT_MAX = 0;
inline constexpr int CCW[] = {0, 1, 2, 1, 2, 0, 2, 0, 1};
extern Vec2 MSAA_TEMPLATE_1[1];
extern Vec2 MSAA_TEMPLATE_2[2];
extern Vec2 MSAA_TEMPLATE_4[4];

Vec2 msaa_template(int factor,int index);


void view_transform(int w, int h, float st_x, float st_y, int& i,
                    int& j);

void invert_view_transform(int w, int h, int i, int j, float& st_x, float& st_y);


void vert_view_transform(const Mat44& mvp, const int w, const int h, VertexAttribute* vertex_attribute,
                                Vec3& alpha_, int& i,
                                int& j);

bool add_fragment(TrianglePrimitive& tri,Context* ctx, L_MATH::Vec<float, 3>& alpha, int i, int j);

bool ray_caster(Context* ctx,  float si, float sj, RayCasterResult* result);
bool ray_caster_bvh(Camera* camera, BVHTree* tree, float si, float sj, RayCasterResult* result);
bool ray_caster_bvh_priqueue(Camera* camera, BVHTree* tree, float si, float sj, RayCasterResult* result);

void rast_tri(TrianglePrimitive& tri, Context* ctx);
void break_huge_triangle(TrianglePrimitive& tri, float area_thr, std::vector<TrianglePrimitive>& result, int& count);

void ray_cast_frag_execute(std::size_t data_begin,std::size_t data_end, void* global_data);
void ray_cast_frag_complete(std::size_t data_begin,std::size_t data_end, void* global_data);

void clear_camera_execute(std::size_t data_begin,std::size_t data_end, void* global_data);
void clear_context_execute(std::size_t data_begin,std::size_t data_end, void* global_data);

void clear_complete(std::size_t data_begin,std::size_t data_end, void* global_data);

void prepare_frag_shader_execute(std::size_t data_begin,std::size_t data_end, void *global_data);
void run_frag_shader_execute(std::size_t data_begin,std::size_t data_end, void* global_data);
void run_frag_shader_complete(std::size_t data_begin,std::size_t data_end, void* global_data);

void rast_tri_execute(std::size_t data_begin,std::size_t data_end, void* global_data);
void rast_tri_complete(std::size_t data_begin,std::size_t data_end, void* global_data);

void mid_filter_execute(std::size_t data_begin,std::size_t data_end, void* global_data);

void mid_filter_complete(std::size_t data_begin,std::size_t data_end, void* global_data);

void execute_mvp(std::size_t data_begin,std::size_t data_end, void* global_data);



#endif //RASTERIZERJOB_H
