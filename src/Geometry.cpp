#include "Geometry.h"

#include "TrianglePrimitive.h"

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
    max = p;
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
void Box<N>::reset()
{
    min = L_MATH::Vec<float, N>(INFINITY);
    max = L_MATH::Vec<float, N>(-INFINITY);
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

Geometry::Geometry(GeometryType sphere): geometry_type(sphere)
{
}

static int geometry_size_array[GeometryCount] = {
    sizeof(TrianglePrimitive),
    sizeof(Sphere),
    sizeof(Quad),
};

template<class T>
static void geometry_build(Geometry* geometry)
{
    new(geometry)T();
}

static void(*geometry_build_array[GeometryCount])(Geometry* geometry)  = {
    geometry_build<TrianglePrimitive>,
    geometry_build<Sphere>,
    geometry_build<Quad>
};

int geometry_size(GeometryType type)
{
    return geometry_size_array[type];
}

void build_geometry(GeometryType type, Geometry* geometry)
{
    geometry_build_array[type](geometry);
}


Sphere::Sphere(): Geometry(SPHERE)
{
}

Sphere::Sphere(const L_MATH::Vec<float, 3>& center, float radius): Geometry(SPHERE), center(center), radius(radius)
{
    geometry_type = SPHERE;
    box = Box<3>(center - Vec3::ONE * radius, center + Vec3::ONE * radius);
}


L_MATH::Vec<float, 3> Sphere::get_normal(const L_MATH::Vec<float, 3>& point)
{
    return (point - center).normalize();
}

L_MATH::Vec<float, 2> Sphere::get_uv(const L_MATH::Vec<float, 3>& point)
{
    auto normalize = (point - center).normalize();
    float beta = asin(normalize[2]);
    float theta = atan2(normalize[1], normalize[0]);
    return {static_cast<float>(beta / PI), static_cast<float>(theta / (2 * PI))};
}

bool Sphere::inside(const L_MATH::Vec<float, 3>& point)
{
    return (point - center).magnitude() <= radius * radius;
}

bool Sphere::intersect(const L_MATH::Vec<float, 3>& point, const L_MATH::Vec<float, 3>& dir,
                       RayCasterResult* result)
{
    Vec3 oc = point - center;
    float a = dir.dot(dir);
    float b = 2.0f * oc.dot(dir);
    float c_val = oc.dot(oc) - radius * radius;
    float discriminant = b * b - 4 * a * c_val;
    if (discriminant >= 0)
    {
        float sqrt_discriminant = std::sqrt(discriminant);
        float t1 = (-b - sqrt_discriminant) / (2.0f * a);
        float t2 = (-b + sqrt_discriminant) / (2.0f * a);
        if (t1 >= 0)
        {
            result->geometry = this;
            result->t = t1;
            return true;
        }
        if (t2 >= 0)
        {
            result->geometry = this;
            result->t = t2;
            return true;
        }
    }
    return false;
}

float Sphere::pdf(const L_MATH::Vec<float, 3>& origin, const L_MATH::Vec<float, 3>& dir)
{
    RayCasterResult result;
    if (!this->intersect(origin, dir, &result))
    {
        return 0;
    }
    auto dis2 = (origin - center).magnitude();
    float cos_theta_max = sqrt(1.f - radius * radius / dis2);
    return 1 / (2 * PI * (1 - cos_theta_max));
}

L_MATH::Vec<float, 3> Sphere::random(const L_MATH::Vec<float, 3>& origin)
{
    auto z_dir = (center - origin).normalize();
    auto dis2 = (origin - center).magnitude();
    float cos_theta_max = sqrt(1.f - radius * radius / dis2);
    float cos_theta = 1 + L_MATH::random() * (cos_theta_max - 1);
    float sin_theta = sqrt(1 - cos_theta * cos_theta);
    float beta = 2 * PI * L_MATH::random();
    Vec3 dir = {sin_theta * cos(beta), sin_theta * sin(beta), cos_theta};
    auto rotate = L_MATH::rotate(L_MATH::FORWARD, z_dir);
    L_MATH::pos3_l_dot_mat33(dir, rotate);
    return dir;
}

void Sphere::transform(const L_MATH::Mat<float, 4, 4>& mat)
{
    L_MATH::pos3_l_dot_mat44(center, mat);
    radius *= mat[0].sqrt_magnitude();
    box = Box<3>(center - Vec3::ONE * radius, center + Vec3::ONE * radius);
}

void Sphere::clone(Geometry* src) const
{
    (*dynamic_cast<Sphere*>(src)) = (*this);
}

Quad::Quad(): Geometry(QUAD)
{
}


Quad::Quad(const L_MATH::Vec<float, 3>& p0, const L_MATH::Vec<float, 3>& p1, const L_MATH::Vec<float, 3>& p2):Geometry(QUAD), p0(p0), p1(p1), p2(p2)
{
    box.expand(p0);
    box.expand(p1);
    box.expand(p2);
    box.expand(p1 - p0 + p2 - p0);
}

L_MATH::Vec<float, 3> Quad::get_normal(const L_MATH::Vec<float, 3>& point)
{
    return L_MATH::cross(p1 - p0, p2 - p1).normalize();
}

L_MATH::Vec<float, 2> Quad::get_uv(const L_MATH::Vec<float, 3>& point)
{
    auto u_dir = p1 - p0; // Edge from p0 to p1
    auto v_dir = p2 - p0; // dge from p0 to p3
    auto local_point = point - p0;
    float u = L_MATH::dot(local_point, u_dir) / L_MATH::dot(u_dir, u_dir);
    float v = L_MATH::dot(local_point, v_dir) / L_MATH::dot(v_dir, v_dir);
    return {u, v};
}

bool Quad::inside(const L_MATH::Vec<float, 3>& p)
{
    auto uv = get_uv(p);
    return uv[0] > 0 && uv[1] > 0 && uv[0] < 1 && uv[1] < 1;

    // auto p3 = p1 - p0 + p2 - p0;
    // Vec3 edge0 = p1 - p0;
    // Vec3 edge1 = p2 - p1;
    // Vec3 edge2 = p3 - p2;
    // Vec3 edge3 = p0 - p3;
    //
    // Vec3 c0 = p - p0;
    // Vec3 c1 = p - p1;
    // Vec3 c2 = p - p2;
    // Vec3 c3 = p - p3;
    //
    // if ((cross(edge0, c0).dot(edge1.cross(c1)) >= 0) &&
    //     (cross(edge1, c1).dot(edge2.cross(c2)) >= 0) &&
    //     (edge2.cross(c2).dot(edge3.cross(c3)) >= 0) &&
    //     (edge3.cross(c3).dot(edge0.cross(c0)) >= 0))
    // {
    //     return true;
    // }
    // return false;
}

bool Quad::intersect(const L_MATH::Vec<float, 3>& point, const L_MATH::Vec<float, 3>& dir,
                     RayCasterResult* result)
{
    Vec3 n = (p1 - p0).cross(p2 - p1);
    float denom = dir.dot(n);

    if (std::abs(denom) < 1e-6)
    {
        return false;
    }

    float t = (p0 - point).dot(n) / denom;
    if (t < 0)
    {
        return false; // Intersection is behind the ray origin
    }
    Vec3 p = point + dir * t;
    if (inside(p))
    {
        result->t = t;
        result->geometry = this;
        return true;
    }
    return false;
}

float Quad::pdf(const L_MATH::Vec<float, 3>& origin, const L_MATH::Vec<float, 3>& dir)
{
    RayCasterResult result;
    if (!intersect(origin,dir,&result))
    {
        return 0;
    }
    Vec3 hit_point = origin + dir * result.t;
    auto dir_length = dir.sqrt_magnitude();
    auto distance2 = result.t * result.t * dir_length * dir_length;
    float area = (L_MATH::cross(p1 - p0, p2 - p1).sqrt_magnitude());
    auto normal = get_normal(hit_point);
    float cose = (L_MATH::dot(normal, dir)) / dir_length;
    return fabs(distance2 / (area * cose));
}

Vec3 Quad::random(const L_MATH::Vec<float, 3>& origin)
{
    auto u = L_MATH::random();
    auto v = L_MATH::random();
    return p0 + (p1 - p0) * u + (p2 - p0) * v - origin;
}

void Quad::transform(const L_MATH::Mat<float, 4, 4>& mat)
{
    L_MATH::pos3_l_dot_mat44(p0, mat);
    L_MATH::pos3_l_dot_mat44(p1, mat);
    L_MATH::pos3_l_dot_mat44(p2, mat);
    box.expand(p0);
    box.expand(p1);
    box.expand(p2);
    box.expand(p1 - p0 + p2 - p0);
}

void Quad::clone(Geometry* src) const
{
    (*dynamic_cast<Quad*>(src)) = (*this);
}






