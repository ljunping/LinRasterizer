//
// Created by Lin on 2024/11/14.
//

#ifndef TRIANGLE_H
#define TRIANGLE_H
#pragma once
#include <vector>
#include "BVHTree.h"
#include "L_math.h"
#include "Geometry.h"

#define MAX_CLIP_VERT_COUNT 8
struct RenderNode;
class Mesh;
struct RayCasterResult;
class DrawCallContext;
struct  VertexInterpolation;

struct TrianglePrimitive : Geometry
{
    TrianglePrimitive();
    Mesh* mesh{};
    int vert_index[3];
    Vec3 v[3];
    Vec3 cache_normal_dir;
    float cache_inv_cross_dir_z;
    Vec3 cache_inv_cross_dir;
    float cache_area;
    bool clipped = false;
    bool discard = false;
    float d;
    Vec3 cache_inv_w;
    Vec3 clip_vertices[MAX_CLIP_VERT_COUNT];
    Vec3 clip_vertices_alpha[MAX_CLIP_VERT_COUNT];
    int clip_vert_count = 0;
    bool ccw() const;
    Vec3 normal() const;
    float area() const;
    bool inside_2D(const Vec3& point) const;
    bool inside(const Vec3& point) override;
    bool is_same_sign(const Vec3& alpha) const;
    float intersect_plane(const Vec3& point, const Vec3& dir) const;
    bool intersect(const L_MATH::Vec<float, 3>& point, const Vec3& dir, RayCasterResult* result) override;
    bool intersect_2D(const L_MATH::Vec<float, 3>& point, RayCasterResult* result);
    L_MATH::Vec<float, 3> get_normal(const L_MATH::Vec<float, 3>& point) override;
    L_MATH::Vec<float, 2> get_uv(const L_MATH::Vec<float, 3>& point) override;
    L_MATH::Vec<float, 3> random(const L_MATH::Vec<float, 3>& origin) override;
    void clone(Geometry* src) const override;
    void transform(const L_MATH::Mat<float, 4, 4>& mat) override;
    float pdf(const L_MATH::Vec<float, 3>& origin, const L_MATH::Vec<float, 3>& dir) override;
    void barycentric2D(const L_MATH::Vec<float, 3>&, L_MATH::Vec<float, 3>& alpha) const;
    void barycentric3D(const L_MATH::Vec<float, 3>&, L_MATH::Vec<float, 3>& alpha) const;
    void update(const Mat44& mat);
    void update_param();
    void clip_2D();

};







#endif //TRIANGLE_H
