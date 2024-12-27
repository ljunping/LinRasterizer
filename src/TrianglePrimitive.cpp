//
// Created by Lin on 2024/11/14.
//

#include "TrianglePrimitive.h"
#include "Camera.h"
#include "debug.h"


void Sutherland_Hodgman(std::vector<Vec3>& clip_plane_normal,
                               std::vector<float>& clip_plane_c,
                               std::vector<Vec3>& ccw_points, std::vector<std::vector<float>>* result)
{
    int clip_size = ccw_points.size();
    std::vector<std::vector<float>>* ccw_points_temps[2];
    std::vector<std::vector<float>> ccw_points_temp_3;
    ccw_points_temps[(clip_plane_normal.size() & 1) ^ 1] = &ccw_points_temp_3;
    ccw_points_temps[clip_plane_normal.size() & 1] = result;
    int temp_index = 0;
    for (int i = 0; i < ccw_points.size(); i++)
    {
        std::vector<float> temp_c(ccw_points.size(), 0);
        temp_c[i] = 1;
        ccw_points_temps[temp_index]->emplace_back(std::move(temp_c));
    }

    auto get_pos = [&ccw_points](std::vector<float>& alpha)
    {
        Vec3 pos;
        for (int i = 0; i < ccw_points.size(); i++)
        {
            pos += ccw_points[i] * alpha[i];
        }
        return pos;
    };

    auto calculate_alpha = [](std::vector<float>& alpha_l, std::vector<float>& alpha_r, float t,
                              std::vector<float>& result)
    {
        float last = 0;
        for (int i = 0; i < alpha_l.size() - 1; ++i)
        {
            result[i] = alpha_l[i] * (1 - t) + alpha_r[i] * t;
            last += result[i];
        }
        result[alpha_l.size() - 1] = 1 - last;
    };

    for (int i = 0; i < clip_plane_normal.size(); ++i)
    {
        auto& p_normal = clip_plane_normal[i];
        float c = clip_plane_c[i];
        auto& ccw_points_temp_1 = *(ccw_points_temps[temp_index]);
        auto& ccw_points_temp_2 = *(ccw_points_temps[temp_index ^ 1]);
        ccw_points_temp_2.clear();
        for (int l = 0; l < ccw_points_temp_1.size(); ++l)
        {
            auto& alpha_l = ccw_points_temp_1[l];
            auto l_p = get_pos(alpha_l);
            bool l_inside = dot(p_normal, l_p) - c < 0;
            auto r = (l + 1) % ccw_points_temp_1.size();
            auto& alpha_r = ccw_points_temp_1[r];
            auto r_p = get_pos(alpha_r);
            // 假定平面方程<0是内部
            bool r_inside = dot(p_normal, r_p) - c < 0;
            if (!l_inside && !r_inside)
            {
                continue;
            }
            if (l_inside)
            {
                ccw_points_temp_2.emplace_back(alpha_l);
            }
            if (!l_inside || !r_inside)
            {
                auto dir = r_p - l_p;
                auto t = intersect_plane(l_p, dir, p_normal, c);
                if (t > 0 && t < 1)
                {
                    std::vector<float> result_l(clip_size, 0);
                    calculate_alpha(alpha_l, alpha_r, t, result_l);
                    ccw_points_temp_2.emplace_back(std::move(result_l));
                }
            }
        }
        temp_index ^= 1;
    }
}


Vec3 TrianglePrimitive::st_box_plane_normal[6] = {
    Vec3{0, 0, 1},
    Vec3{0, 0, -1},
    Vec3{0, 1, 0},
    Vec3{0, -1, 0},
    Vec3{1, 0, 0},
    Vec3{-1, 0, 0}
};
Box<3> TrianglePrimitive::st_box(Vec3{-1, -1, -1},Vec3{1, 1, 1});



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
    alpha.data[2] = 1 - alpha.data[0] - alpha.data[1];
}

// void TrianglePrimitive::update(const L_MATH::Mat<float, 4, 4>& mat)
// {
//     for (int i = 0; i < 3; ++i)
//     {
//         Vec4& _v4 = static_cast<L_MATH::Vec<float, 4>&>(v[i]);
//         _v4[3] = 1;
//         _v4 = _v4.mul_transpose(mat);
//         inv_w[i] = 1 / _v4[3];
//         _v4 /= _v4[3];
//     }
//     update_param();
// }

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
    inv_w[0] = 1 / v[0][3];
    inv_w[1] = 1 / v[1][3];
    inv_w[2] = 1 / v[2][3];
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
    clip_vert_count = 0;
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
    std::vector<Vec3> clip_plane_normal;
    std::vector<float> clip_plane_c(6, 1);
    for (int i = 0; i < 6; ++i)
    {
        clip_plane_normal.emplace_back( st_box_plane_normal[i]);
    }
    Sutherland_Hodgman(clip_plane_normal, clip_plane_c, _clip_vertices, &clip_alphas);
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

void TrianglePrimitive::reset()
{

}
