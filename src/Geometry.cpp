#include "Geometry.h"

#include "TrianglePrimitive.h"



Vec3 VERTICES_INDEX[] = {
    {+1, +1, +1}, {+1, +1, -1}, {+1, -1, +1}, {+1, -1, -1},
    {-1, +1, +1}, {-1, +1, -1}, {-1, -1, +1}, {-1, -1, -1}
};

int BOX_3_TRI_INDEX[] = {
    0, 4, 2, 2, 4, 6,  // Front face
    1, 3, 5, 3, 7, 5,  // Back face
    4, 5, 6, 6, 5, 7,  // Left face
    0, 2, 1, 1, 2, 3,  // Right face
    0, 1, 4, 4, 1, 5,  // Top face
    2, 6, 3, 3, 6, 7   // Bottom face
};




template <int N>
L_MATH::Vec<float, N> Box<N>::center() const
{
    return (min + max) * 0.5f;
}

template <int N>
Box<N>::Box()
{
    min = L_MATH::Vec<float, N>(INFINITY);
    max = L_MATH::Vec<float, N>(-INFINITY);
}

template<int N>
Box<N>::Box(const L_MATH::Vec<float, N>& p)
{
    min = p;
    max = p;
}

template <int N>
Box<N>::Box(const L_MATH::Vec<float, N>& p, const L_MATH::Vec<float, N>& q)
{
    min = p;
    max = q;
    expand(q);
}

template <int N>
std::vector<L_MATH::Vec<float, N>> Box<N>::get_vertices() const
{
    std::vector<L_MATH::Vec<float, N>> vertices;
    auto center = this->center();
    auto xyz = this->max - center;
    for (auto& vertices_index : VERTICES_INDEX)
    {
        vertices.emplace_back(xyz[0] * vertices_index[0], xyz[1] * vertices_index[1], xyz[2] * vertices_index[2]);
    }
    return vertices;
}



template <int N>
void Box<N>::expand(const L_MATH::Vec<float, N>& v)
{
    min = fmin(min, v);
    max = fmax(max, v);
}

template <int N>
void Box<N>::expand(const Box<N>& v)
{
    min = fmin(min, v.min);
    max = fmax(max, v.max);
}

template <int N>
template <int M>
bool Box<N>::inside(const L_MATH::Vec<float, N>& point) const
{
    static_assert(N >= M, "N must be greater than M");
    if constexpr (M == 1)
    {
        return point.data[0] > min.data[0] && point.data[0] < max.data[0];
    }
    if constexpr (M == 2)
    {
        return point.data[0] > min.data[0] && point.data[0] < max.data[0]
            && point.data[1] > min.data[1] && point.data[1] < max.data[1];
    }
    else if constexpr (N == 3)
    {
        return point.data[0] > min.data[0] && point.data[0] < max.data[0]
            && point.data[1] > min.data[1] && point.data[1] < max.data[1]
            && point.data[2] > min.data[2] && point.data[2] < max.data[2];
    }
    for (int i = 0; i < M; ++i)
    {
        if (point.data[i] > min.data[i] && point.data[i] < max.data[i])
        {
            return false;
        }
    }
    return true;

}

template <int N>
bool Box<N>::intersect(const L_MATH::Vec<float, N>& origin,const L_MATH::Vec<float, N>& dir,  float& tMin, float& tMax) const
{
    tMin = -INFINITY;
    tMax = INFINITY;
    for (int i = 0; i < N; ++i)
    {
        if (!L_MATH::is_zero(dir[i]))
        {
            float invD = 1.0f / dir[i];
            float t0 = (min[i] - origin[i]) * invD;
            float t1 = (max[i] - origin[i]) * invD;
            if (invD < 0.0f) std::swap(t0, t1);
            tMin = t0 > tMin ? t0 : tMin;
            tMax = t1 < tMax ? t1 : tMax;
            if (tMax < tMin) return false;
        }
        else
        {
            if (origin[i] < min[i] || origin[i] > max[i])
            {
                return false;
            }
        }
    }
    return tMax > 0;
}
template class Box<2>;
template bool Box<2>::inside<1>(const L_MATH::Vec<float, 2>& point) const;
template bool Box<2>::inside<2>(const L_MATH::Vec<float, 2>& point) const;


template class Box<3>;
template bool Box<3>::inside<1>(const L_MATH::Vec<float, 3>& point) const;
template bool Box<3>::inside<2>(const L_MATH::Vec<float, 3>& point) const;
template bool Box<3>::inside<3>(const L_MATH::Vec<float, 3>& point) const;


Plane::Plane(const L_MATH::Vec<float, 3>& normal, const L_MATH::Vec<float, 3>& p)
{
    this->normal = normal.normalize();
    c = dot(this->normal, p);
}

Plane::Plane(const L_MATH::Vec<float, 3>& p1, const L_MATH::Vec<float, 3>& p2, const L_MATH::Vec<float, 3>& p3)
{
    this->normal = cross(p2 - p1, p3 - p1);
    this->normal.normalized();
    c = dot(this->normal, p1);
}

float intersect_plane(const L_MATH::Vec<float, 3>& point, const L_MATH::Vec<float, 3>& dir,
    const L_MATH::Vec<float, 3>& plane_normal, float c)
{
    float t = -(L_MATH::dot(plane_normal, point) - c) / dot(dir, plane_normal);
    return t;
}

bool Frustum::ray_intersect_frustum(const L_MATH::Vec<float, 3>& pos, const L_MATH::Vec<float, 3>& dir) const
{
    auto planes = get_frustum_planes();
    int count = 0;
    for (auto plane : planes)
    {
        auto t = intersect_plane(pos, dir, plane.normal, plane.c);
        if (point_in_frustum(pos + dir * t))
        {
            count++;
        }
        if (count >= 2)
        {
            return true;
        }
    }
    return false;
}

bool Frustum::point_in_frustum(const L_MATH::Vec<float, 3>& pos) const
{
    auto proj_mat = get_proj_mat();
    auto proj_pos = L_MATH::pos3_dot_mat44(pos, proj_mat);
    for (int i = 0; i < 3; ++i)
    {
        if (proj_pos[i] < -1 - EPSILON || proj_pos[i] >= 1 + EPSILON)
        {
            return false;
        }
    }
    return true;
}

bool Frustum::frustum_in_frustum(const Frustum& frustum1,const L_MATH::Mat<float, 4, 4>& vm_mat) const
{
    auto mvp = get_proj_mat() * vm_mat;
    auto vecs = frustum1.get_vertices();
    std::vector<L_MATH::Vec<float, 3>> verts(3);
    std::vector<std::vector<float>> clip_result;
    for (int i = 0; i < 12; ++i)
    {
        clip_result.clear();
        verts[0] = vecs[BOX_3_TRI_INDEX[i * 3]];
        verts[1] = vecs[BOX_3_TRI_INDEX[i * 3 + 1]];
        verts[2] = vecs[BOX_3_TRI_INDEX[i * 3 + 2]];
        L_MATH::pos3_l_dot_mat44(verts[0], mvp);
        L_MATH::pos3_l_dot_mat44(verts[1], mvp);
        L_MATH::pos3_l_dot_mat44(verts[2], mvp);
        bool early_finish = false;
        for (int j = 0; j < 3; ++j)
        {
            if (verts[0][j] < -1 && verts[1][j] < -1 && verts[2][j] < -1)
            {
                early_finish = true;
                break;
            }
            if (verts[0][j] > 1 && verts[1][j] > 1 && verts[2][j] > 1)
            {
                early_finish = true;
                break;
            }
        }
        if (early_finish)
        {
            continue;
        }
        Sutherland_Hodgman(ST_BOX_PLANE_NORMAL, ST_BOX_PLANE_C, verts, &clip_result);
        if (!clip_result.empty())
        {
            return true;
        }
    }
    return false;
}


bool Frustum::box_in_frustum(const Box<3>& box, const Mat44& vm_mat) const
{
    auto mvp = get_proj_mat() * vm_mat;
    auto vecs = box.get_vertices();
    std::vector<L_MATH::Vec<float, 3>> verts(3);
    std::vector<std::vector<float>> clip_result;
    for (int i = 0; i < 12; ++i)
    {
        clip_result.clear();
        verts[0] = vecs[BOX_3_TRI_INDEX[i * 3]];
        verts[1] = vecs[BOX_3_TRI_INDEX[i * 3 + 1]];
        verts[2] = vecs[BOX_3_TRI_INDEX[i * 3 + 2]];
        L_MATH::pos3_l_dot_mat44(verts[0], mvp);
        L_MATH::pos3_l_dot_mat44(verts[1], mvp);
        L_MATH::pos3_l_dot_mat44(verts[2], mvp);
        bool early_finish = false;
        for (int j = 0; j < 3; ++j)
        {
            if (verts[0][j] < -1 && verts[1][j] < -1 && verts[2][j] < -1)
            {
                early_finish = true;
                break;
            }
            if (verts[0][j] > 1 && verts[1][j] > 1 && verts[2][j] > 1)
            {
                early_finish = true;
                break;
            }
        }
        if (early_finish)
        {
            continue;
        }
        Sutherland_Hodgman(ST_BOX_PLANE_NORMAL, ST_BOX_PLANE_C, verts, &clip_result);
        if (!clip_result.empty())
        {
            return true;
        }
    }
    return false;
}

L_MATH::Mat<float, 4, 4> Frustum::get_proj_mat() const
{
    return L_MATH::project(near, far, fov, aspect_ratio);
}

std::vector<L_MATH::Vec<float, 3>> Frustum::get_vertices() const
{
    std::vector<Vec3> vecs;
    float tan_fov2 = tan(L_MATH::deg2rad(fov) / 2);
    float half_height = tan_fov2 * near;
    float half_width = half_height * aspect_ratio;
    float far_height = half_height * far / near;
    float far_width = half_width * far / near;
    vecs.emplace_back(half_width, half_height, -near);
    vecs.emplace_back(far_width, far_height, -far);
    vecs.emplace_back(half_width, -half_height, -near);
    vecs.emplace_back(far_width, -far_height, -far);
    vecs.emplace_back(-half_width, half_height, -near);
    vecs.emplace_back(-far_width, far_height, -far);
    vecs.emplace_back(-half_width, -half_height, -near);
    vecs.emplace_back(-far_width, -far_height, -far);
    return vecs;
}






std::vector<Plane> Frustum::get_frustum_planes() const
{
    std::vector<Plane> planes;
    float tan_fov2 = tan(fov / 2);
    float half_height = tan_fov2 * near;
    float half_width = half_height * aspect_ratio;
    planes.emplace_back(Vec3{0, 0, 0},Vec3{half_width, half_height, -near},Vec3{-half_width, half_height, -near});
    planes.emplace_back(Vec3{0, 0, 0},Vec3{half_width, -half_height, -near},Vec3{half_width, half_height, -near});
    planes.emplace_back(Vec3{0, 0, 0},Vec3{-half_width, half_height, -near},Vec3{-half_width, -half_height, -near});
    planes.emplace_back(Vec3{0, 0, 0},Vec3{half_width, -half_height, -near},Vec3{-half_width, -half_height, -near});
    planes.emplace_back(Vec3{half_width, half_height, -near},Vec3{-half_width, half_height, -near},
                        Vec3{-half_width, -half_height, -near});
    planes.emplace_back(Vec3{half_width, half_height, -far},Vec3{half_width, -half_height, -far},
                        Vec3{-half_width, -half_height, -far});
    return planes;
}



void Sutherland_Hodgman(const std::vector<L_MATH::Vec<float, 3>>& clip_plane_normal, std::vector<float>& clip_plane_c,
                        std::vector<L_MATH::Vec<float, 3>>& ccw_points, std::vector<std::vector<float>>* result)
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
