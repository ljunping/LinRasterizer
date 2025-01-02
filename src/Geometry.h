//
// Created by Lin on 2024/11/14.
//

#ifndef Box_H
#define Box_H
#include"L_math.h"
#include <queue>

template<int N>
class alignas(16) Box;

template<int N>
class alignas(16) Box
{
public:
    L_MATH::Vec<float, N> min;
    L_MATH::Vec<float, N> max;
    L_MATH::Vec<float, N> center() const;
    Box();
    explicit Box(const L_MATH::Vec<float, N>& p);

    Box(const L_MATH::Vec<float, N>& p, const L_MATH::Vec<float, N>& q);
    //NROV优化，直接在调用处构造
    std::vector<L_MATH::Vec<float,N>> get_vertices() const;
    void expand(const L_MATH::Vec<float, N>& v);
    void expand(const Box<N>& v);
    template <int M>
    bool inside(const L_MATH::Vec<float, N>& point) const;
    bool intersect( const L_MATH::Vec<float, N>& origin,const L_MATH::Vec<float, N>& dir,  float& tMin, float& tMax) const;
};


// 平面方程 plane_normal*x-c=0
struct Plane
{
    Vec3 normal;
    float c;

public:
    Plane(const Vec3& normal, const Vec3& p);

    Plane(const Vec3& p1, const Vec3& p2, const Vec3& p3);
};


float intersect_plane(const L_MATH::Vec<float, 3>& point, const Vec3& dir, const Vec3& plane_normal, float c);

struct Frustum
{
    float near;
    float far;
    float fov;
    float aspect_ratio;
    Frustum(): near(0), far(0), fov(0), aspect_ratio(0)
    {
    }

    Frustum(float near, float far, float fov, float aspect_ratio)
    {
        this->near = near;
        this->far = far;
        this->fov = fov;
        this->aspect_ratio = aspect_ratio;
    }
    std::vector<Plane> get_frustum_planes() const;
    bool ray_intersect_frustum(const L_MATH::Vec<float, 3>& pos, const L_MATH::Vec<float, 3>& dir) const;
    bool point_in_frustum(const L_MATH::Vec<float, 3>& pos) const;
    bool box_in_frustum(const Box<3>& box, const L_MATH::Mat<float, 4, 4>& vm) const;
    Mat44 get_proj_mat() const;
    std::vector<Vec3> get_vertices() const;
    bool frustum_in_frustum(const Frustum& frustum1, const L_MATH::Mat<float, 4, 4>& vm_mat) const;

};

inline std::vector<Vec3> ST_BOX_PLANE_NORMAL = {
    Vec3{0, 0, 1},
    Vec3{0, 0, -1},
    Vec3{0, 1, 0},
    Vec3{0, -1, 0},
    Vec3{1, 0, 0},
    Vec3{-1, 0, 0}
};
inline std::vector<float> ST_BOX_PLANE_C(6, 1);

inline Box<3> ST_BOX(Vec3{-1, -1, -1},Vec3{1, 1, 1});


void Sutherland_Hodgman(const std::vector<Vec3>& clip_plane_normal,
                               std::vector<float>& clip_plane_c,
                               std::vector<Vec3>& ccw_points, std::vector<std::vector<float>>* result);

#endif //Box_H
