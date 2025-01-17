//
// Created by Lin on 2024/11/14.
//

#include "TrianglePrimitive.h"

#include "BVHTree.h"
#include "Camera.h"
#include "debug.h"
#include "Mesh.h"


TrianglePrimitive::TrianglePrimitive(): Geometry(TRI)
{
}


bool TrianglePrimitive::ccw() const
{
    return cache_inv_cross_dir_z > 0;
}

L_MATH::Vec<float, 3> TrianglePrimitive::normal() const
{
    Vec3 cross_dir = L_MATH::cross(v[1] - v[0], v[2] - v[1]);
    cross_dir.normalized();
    return cross_dir;
}

float TrianglePrimitive::area() const
{
    Vec3 cross_dir = L_MATH::cross(v[1] - v[0], v[2] - v[1]);
    return cross_dir.sqrt_magnitude() / 2;
}

bool TrianglePrimitive::inside_2D(const L_MATH::Vec<float, 3>& point) const
{
    if (!box.inside<2>(point))
    {
        return false;
    }
    Vec3 vec;
    barycentric2D(point, vec);
    return is_same_sign(vec);
}

bool TrianglePrimitive::inside(const L_MATH::Vec<float, 3>& point)
{
    if (!box.inside<3>(point))
    {
        return false;
    }
    Vec3 vec;
    barycentric3D(point, vec);
    return is_same_sign(vec);
}


bool TrianglePrimitive::is_same_sign(const L_MATH::Vec<float, 3>& alpha) const
{
    return ((alpha.data[0] >= 0) && (alpha.data[1] >= 0) && (alpha.data[2] >= 0));
}

float TrianglePrimitive::intersect_plane(const L_MATH::Vec<float, 3>& point,const Vec3& dir) const
{
    float t =  ::intersect_plane(point, dir, cache_normal_dir, d);;
    return t;
}
//三角形基元已经投影变换
bool TrianglePrimitive::intersect(const L_MATH::Vec<float, 3>& point, const L_MATH::Vec<float, 3>& dir,
                                  RayCasterResult* result)
{
    result->t = intersect_plane(point, dir);
    barycentric3D(point + dir * result->t, result->alpha);
    result->geometry = this;
    result->triangle = this;
    return is_same_sign(result->alpha);
}


bool TrianglePrimitive::intersect_2D(const L_MATH::Vec<float, 3>& point, RayCasterResult* result)
{
    barycentric2D(point, result->alpha);
    result->geometry = this;
    result->triangle = this;
    return is_same_sign(result->alpha);
}

L_MATH::Vec<float, 3> TrianglePrimitive::get_normal(const L_MATH::Vec<float, 3>& point)
{
    return normal();
}

L_MATH::Vec<float, 2> TrianglePrimitive::get_uv(const L_MATH::Vec<float, 3>& point)
{
    Vec3 alpha;
    barycentric3D(point, alpha);
    Vec2 uv0, uv1, uv2;
    this->ctx->get_vert_attribute_value(this->gl_vert_index[0], UV, uv0);
    this->ctx->get_vert_attribute_value(this->gl_vert_index[1], UV, uv1);
    this->ctx->get_vert_attribute_value(this->gl_vert_index[2], UV, uv2);
    return uv0 * alpha[0] + uv1 * alpha[1] + uv2 * alpha[2];
}


L_MATH::Vec<float, 3> TrianglePrimitive::random(const L_MATH::Vec<float, 3>& origin)
{
    float u = L_MATH::random(), v = L_MATH::random();
    if ((u + v) > 1)
    {
        u = 1 - u;
        v = 1 - v;
    }
    return this->v[0] * u + this->v[1] * v + this->v[2] * (1 - u - v) - origin;
}


void TrianglePrimitive::clone(Geometry* src) const
{
    (*dynamic_cast<TrianglePrimitive*>(src)) = (*this);
}


void TrianglePrimitive::transform(const L_MATH::Mat<float, 4, 4>& mat)
{
    update(mat);
}

float TrianglePrimitive::pdf(const L_MATH::Vec<float, 3>& origin, const L_MATH::Vec<float, 3>& dir)
{
    RayCasterResult result;
    if (!intersect(origin, dir, &result))
    {
        return 0;
    }
    auto dir_len = dir.sqrt_magnitude();
    auto distance2 = result.t * result.t * dir_len * dir_len;
    float cose = fabs(dot(dir, cache_normal_dir) / dir_len);
    return fabs(distance2 / (cose * cache_area));
}

void TrianglePrimitive::barycentric2D(const L_MATH::Vec<float, 3>& p,Vec3& alpha) const
{
    float c;
    L_MATH::cross_opt_3(v[1] - p, v[2] - v[1], c);
    alpha.data[0] = c * cache_inv_cross_dir_z;
    L_MATH::cross_opt_3(v[2] - p, v[0] - v[2], c);
    alpha.data[1] = c * cache_inv_cross_dir_z;
    alpha.data[2] = 1 - alpha.data[0] - alpha.data[1];
}

void TrianglePrimitive::barycentric3D(const L_MATH::Vec<float, 3>& p, L_MATH::Vec<float, 3>& alpha) const
{
    auto _v0 = L_MATH::cross(v[1] - p, v[2] - v[1]);
    alpha.data[0] = dot(_v0, cache_inv_cross_dir);
    auto _v1 = L_MATH::cross(v[2] - p, v[0] - v[2]);
    alpha.data[1] = dot(_v1, cache_inv_cross_dir);
    alpha.data[2] = 1 - alpha.data[0] - alpha.data[1];
}


void TrianglePrimitive::update(const L_MATH::Mat<float, 4, 4>& mat)
{
    for (int i = 0; i < 3; ++i)
    {
        Vec4& _v4 = static_cast<L_MATH::Vec<float, 4>&>(v[i]);
        _v4[3] = 1;
        _v4 = _v4.mul_transpose(mat);
        cache_inv_w[i] = 1 / _v4[3];
        _v4 /= _v4[3];
    }
    update_param();
}

void TrianglePrimitive::update_param()
{
    box.max = v[0];
    box.min = v[0];
    box.expand(v[1]);
    box.expand(v[2]);
    Vec3 cross_dir = L_MATH::cross(v[1] - v[0], v[2] - v[1]);
    cache_area = cross_dir.sqrt_magnitude() / 2;
    cache_normal_dir = cross_dir.normalize();
    d = cache_normal_dir.dot(v[0]);
    cache_inv_w[0] = 1 / v[0][3];
    cache_inv_w[1] = 1 / v[1][3];
    cache_inv_w[2] = 1 / v[2][3];
    cache_inv_cross_dir_z = 1 / (cross_dir[2]);
    cache_inv_cross_dir = cache_normal_dir * (1 / cross_dir.sqrt_magnitude());
    clipped = false;
    out_near = false;
}


void TrianglePrimitive::clip_st_box()
{
    if (clipped)
    {
        return;
    }
    clipped = true;
    clip_vert_count = 0;
    Vec3 min = fmin(ST_BOX.min, box.min);
    Vec3 max = fmax(ST_BOX.max, box.max);
    //两个box完全不相交
    if (max[0] - min[0] > ST_BOX.max[0] - ST_BOX.min[0] + box.max[0] - box.min[0]
        && max[1] - min[1] > ST_BOX.max[1] - ST_BOX.min[1] + box.max[1] - box.min[1]
        && max[2] - min[2] > ST_BOX.max[2] - ST_BOX.min[2] + box.max[2] - box.min[2])
    {
        return;
    }
    bool need_clip = false;
    bool is_inside[3];
    for (int i = 0; i < 3; ++i)
    {
        is_inside[i]=ST_BOX.inside<3>(v[i]);
        if (!is_inside[i])
        {
            need_clip = true;
            break;
        }
    }
    std::vector<Vec3> _clip_vertices;
    clip_vertices[0] = v[0];
    clip_vertices[1] = (v[1]);
    clip_vertices[2] = (v[2]);
    _clip_vertices.emplace_back(v[0]);
    _clip_vertices.emplace_back(v[1]);
    _clip_vertices.emplace_back(v[2]);
    clip_vert_count = 3;
    if (!need_clip)
    {
        clip_vertices_alpha[0] = (Vec3{1, 0, 0});
        clip_vertices_alpha[1] = (Vec3{0, 1, 0});
        clip_vertices_alpha[2] = (Vec3{0, 0, 1});
        return;
    }
    std::vector<std::vector<float>> clip_alphas;
    Sutherland_Hodgman(ST_BOX_PLANE_NORMAL, ST_BOX_PLANE_C, _clip_vertices, &clip_alphas);
    RUNTIME_ASSERT(clip_alphas.size()<=MAX_CLIP_VERT_COUNT,"clip_alphas.size()<=MAX_CLIP_VERT_COUNT");
    for (int i = 0; i < clip_alphas.size(); ++i)
    {
        auto& clip_alpha = clip_alphas[i];
        clip_vertices_alpha[i][0] = clip_alpha[0];
        clip_vertices_alpha[i][1] = clip_alpha[1];
        clip_vertices_alpha[i][2] = clip_alpha[2];
        clip_vertices[i] = v[0] * clip_alpha[0] + v[1] * clip_alpha[1] + v[2] * clip_alpha[2];
    }
    clip_vert_count = clip_alphas.size();
}


