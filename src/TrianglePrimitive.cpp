//
// Created by Lin on 2024/11/14.
//

#include "TrianglePrimitive.h"
#include <__format/format_functions.h>
#include "Camera.h"
#include "debug.h"
#include "Rasterizer.h"

Vec3 TrianglePrimitive::st_box_plane_normal[6] = {
    Vec3{0, 0, 1},
    Vec3{0, 0, -1},
    Vec3{0, 1, 0},
    Vec3{0, -1, 0},
    Vec3{1, 0, 0},
    Vec3{-1, 0, 0}
};
Box3D TrianglePrimitive::st_box(Vec3{-1, -1, -1},Vec3{1, 1, 1});


TrianglePrimitive::~TrianglePrimitive()
{
}

TrianglePrimitive::TrianglePrimitive(VertexAttribute& v0, VertexAttribute& v1, VertexAttribute& v2)
{
    vert[0] = v0;
    vert[1] = v1;
    vert[2] = v2;
    v0.get_attribute_value(0, v[0]);
    v1.get_attribute_value(0, v[1]);
    v2.get_attribute_value(0, v[2]);
    inv_w = Vec3::ONE;
}

bool TrianglePrimitive::ccw() const
{
    return inv_cross_dir_z > 0;
}

L_MATH::Vec<float, 3> TrianglePrimitive::normal() const
{
    return normal_dir;
}

float TrianglePrimitive::area() const
{
    return cache_area;
}

bool TrianglePrimitive::inside(const L_MATH::Vec<float, 3>& point) const
{
    if (!box.inside<2>(point))
    {
        return false;
    }
    Vec3 vec;
    barycentric(point, vec);
    return is_same_sign(vec);
}


bool TrianglePrimitive::is_same_sign(const L_MATH::Vec<float, 3>& alpha) const
{
    auto ccw1 = ccw();
    return (ccw1 && (alpha.data[0] >= 0) && (alpha.data[1] >= 0) && (alpha.data[2] >= 0)) ||
        (!ccw1 && (alpha.data[0] <= 0) && (alpha.data[1] <= 0) && (alpha.data[2] <= 0));
}

float TrianglePrimitive::intersect_plane(const L_MATH::Vec<float, 3>& point,const Vec3& dir) const
{
    float t =  L_MATH::intersect_plane(point, dir, normal_dir, d);;
    return t;
}
//三角形基元已经投影变换
bool TrianglePrimitive::intersect_3D(const L_MATH::Vec<float, 3>& point, const L_MATH::Vec<float, 3>& dir,
    RayCasterResult* result)
{
    barycentric(point, result->alpha);
    result->triangle = this;
    return is_same_sign(result->alpha);
    // result->t = intersect_plane(point, dir);
    // barycentric(point + dir * result->t, result->alpha);
    // result->triangle = this;
    // return is_same_sign(result->alpha);
}

bool TrianglePrimitive::intersect_2D(const L_MATH::Vec<float, 3>& point, RayCasterResult* result)
{
    barycentric(point, result->alpha);
    result->triangle = this;
    return is_same_sign(result->alpha);
}


void TrianglePrimitive::barycentric(const L_MATH::Vec<float, 3>& p,Vec3& alpha) const
{
    float c;
    L_MATH::cross_opt_3(v[1] - p, v[2] - v[1], c);
    alpha.data[0] = c * inv_cross_dir_z;
    L_MATH::cross_opt_3(v[2] - p, v[0] - v[2], c);
    alpha.data[1] = c * inv_cross_dir_z;
    // L_MATH::cross_opt_3(v[0] - p, v[1] - v[0], c);
    // alpha.data[2] = c * inv_cross_dir_z;
    alpha.data[2] = 1 - alpha.data[0] - alpha.data[1];
}

void TrianglePrimitive::update(const L_MATH::Mat<float, 4, 4>& mat)
{
    for (int i = 0; i < 3; ++i)
    {
        Vec4 v4 = Vec4(mat * Vec4(v[i], 1));
        v[i] = Vec3(v4) / v4[3];
        inv_w[i] = 1 / v4[3];
    }
    update_param();
}

void TrianglePrimitive::update_param()
{
    discard = false;
    box.max = v[0];
    box.min = v[0];
    box.expand(v[1]);
    box.expand(v[2]);
    cross_dir = L_MATH::cross(v[1] - v[0], v[2] - v[1]);
    cache_area = cross_dir.sqrt_magnitude() / 2;
    normal_dir = cross_dir.normalize();
    d = normal_dir.dot(v[0]);
    if (L_MATH::is_zero(cross_dir[2]) || L_MATH::is_zero(normal_dir[2]))
    {
        discard = true;
        return;
    }
    inv_cross_dir_z = 1 / cross_dir[2];
    inv_normal_dir_z = 1 / normal_dir[2];
    clipped = false;
}


void TrianglePrimitive::clip()
{
    if (clipped)
    {
        return;
    }
    clipped = true;
    clip_vertices_alpha.clear();
    Vec3 min = fmin(st_box.min, box.min);
    Vec3 max = fmax(st_box.max, box.max);
    //两个box完全不相交
    if (max[0] - min[0] > st_box.max[0] - st_box.min[0] + box.max[0] - box.min[0]
        && max[1] - min[1] > st_box.max[1] - st_box.min[1] + box.max[1] - box.min[1]
        && max[2] - min[2] > st_box.max[2] - st_box.min[2] + box.max[2] - box.min[2])
    {
        return;
    }
    bool need_clip = false;
    bool is_inside[3];
    for (int i = 0; i < 3; ++i)
    {
        is_inside[i]=st_box.inside<3>(v[i]);
        if (!is_inside[i])
        {
            need_clip = true;
            break;
        }
    }
    clip_vertices.emplace_back(v[0]);
    clip_vertices.emplace_back(v[1]);
    clip_vertices.emplace_back(v[2]);
    if (!need_clip)
    {
        clip_vertices_alpha.emplace_back(Vec3{1, 0, 0});
        clip_vertices_alpha.emplace_back(Vec3{0, 1, 0});
        clip_vertices_alpha.emplace_back(Vec3{0, 0, 1});
        return;
    }
    std::vector<std::vector<float>> clip_alphas;
    std::vector<Vec3> clip_plane_normal;
    std::vector<float> clip_plane_c(6, 1);
    for (int i = 0; i < 6; ++i)
    {
        clip_plane_normal.emplace_back( st_box_plane_normal[i]);
    }
    L_MATH::Sutherland_Hodgman(clip_plane_normal, clip_plane_c, clip_vertices, &clip_alphas);
    clip_vertices.resize(clip_alphas.size());
    clip_vertices_alpha.resize(clip_alphas.size());
    for (int i = 0; i < clip_alphas.size(); ++i)
    {
        auto& clip_alpha = clip_alphas[i];
        clip_vertices_alpha[i][0] = clip_alpha[0];
        clip_vertices_alpha[i][1] = clip_alpha[1];
        clip_vertices_alpha[i][2] = clip_alpha[2];
        clip_vertices[i] = v[0] * clip_alpha[0] + v[1] * clip_alpha[1] + v[2] * clip_alpha[2];
    }
}

void TrianglePrimitive::reset()
{

}
