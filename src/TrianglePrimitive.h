//
// Created by Lin on 2024/11/14.
//

#ifndef TRIANGLE_H
#define TRIANGLE_H
#pragma once
#include <vector>
#include "L_math.h"
#include "Mesh.h"
#include "Box3D.h"

class VertexAttribute;

class alignas(16) TrianglePrimitive:public BVHInterface
{
public:
    static Box3D st_box;
    static Vec3 st_box_plane_normal[6];
    VertexAttribute vert[3];
    TrianglePrimitive() = default;
    ~TrianglePrimitive();
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
    std::vector<Vec3> clip_vertices;
    std::vector<Vec3> clip_vertices_alpha;
    TrianglePrimitive(VertexAttribute& v0, VertexAttribute& v1, VertexAttribute& v2);
    bool ccw() const;
    Vec3 normal() const;
    float area() const;
    bool inside(const Vec3& point) const;
    bool is_same_sign(const Vec3& alpha) const;
    float intersect_plane(const Vec3& point,const Vec3& dir) const;
    bool intersect_3D(const L_MATH::Vec<float, 3>& point, const Vec3& dir, RayCasterResult* result) override;
    bool intersect_2D(const L_MATH::Vec<float, 3>& point, RayCasterResult* result) override;
    void barycentric(const L_MATH::Vec<float, 3>&, L_MATH::Vec<float, 3>& alpha) const;
    void update(const Mat44& mat);
    void update_param();
    void clip();
    void reset();
};





#endif //TRIANGLE_H
