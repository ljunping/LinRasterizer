//
// Created by Lin on 2024/11/14.
//

#ifndef TRIANGLE_H
#define TRIANGLE_H
#pragma once
#include <vector>
#include "L_math.h"
#include "Mesh.h"
#include "Geometry.h"

#define MAX_CLIP_VERT_COUNT 8
struct RayCasterResult;
class DrawCallContext;
struct  VertexInterpolation;
//这个类纯数据类，不要继承其他类，不能执行析构函数
struct TrianglePrimitive
{
    static Box<3> st_box;
    static Vec3 st_box_plane_normal[6];
    Box<3> box;
    TrianglePrimitive() = default;
    int id{};
    Mesh* mesh{};
    int vert_index[3];
    Vec3 v[3];
    Vec3 normal_dir;
    Vec3 cross_dir;
    float inv_cross_dir_z;
    float inv_normal_dir_z;
    float cache_area;
    bool clipped=false;
    bool discard=false;
    float d;
    Vec3 inv_w;
    Vec3 clip_vertices[MAX_CLIP_VERT_COUNT];
    Vec3 clip_vertices_alpha[MAX_CLIP_VERT_COUNT];
    int clip_vert_count = 0;
    bool ccw() const;
    Vec3 normal() const;
    float area() const;
    bool inside(const Vec3& point) const;
    bool is_same_sign(const Vec3& alpha) const;
    float intersect_plane(const Vec3& point,const Vec3& dir) const;
    bool intersect_3D(const L_MATH::Vec<float, 3>& point, const Vec3& dir, RayCasterResult* result);
    bool intersect_2D(const L_MATH::Vec<float, 3>& point, RayCasterResult* result);
    void barycentric(const L_MATH::Vec<float, 3>&, L_MATH::Vec<float, 3>& alpha) const;
    void update(const Mat44& mat);
    void update_param();
    void clip();
    void reset();
};







#endif //TRIANGLE_H
