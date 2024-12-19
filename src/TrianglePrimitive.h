//
// Created by Lin on 2024/11/14.
//

#ifndef TRIANGLE_H
#define TRIANGLE_H
#pragma once
#include <vector>
#include "L_math.h"
#include "Mesh.h"
#include "Box.h"

#define MAX_CLIP_VERT_COUNT 8
struct  VertexAttribute;
//这个类纯数据类，不要继承其他类，不能执行析构函数
struct  alignas(16) TrianglePrimitive
{
    static Box<3> st_box;
    static Vec3 st_box_plane_normal[6];
    Box<3> box;
    VertexAttribute vert[3];
    TrianglePrimitive() = default;
    int id;
    alignas(16) Vec3 v[3];
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
    TrianglePrimitive(VertexAttribute& v0, VertexAttribute& v1, VertexAttribute& v2);
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
