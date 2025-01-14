//
// Created by Lin on 2024/11/14.
//

#ifndef Box_H
#define Box_H
#include"L_math.h"
#include <queue>

#include "CommonMacro.h"

struct Geometry;
struct RayCasterResult;
inline Vec3 VERTICES_INDEX[] = {
    {+1, +1, +1}, {+1, +1, -1}, {+1, -1, +1}, {+1, -1, -1},
    {-1, +1, +1}, {-1, +1, -1}, {-1, -1, +1}, {-1, -1, -1}
};

inline int BOX_3_TRI_INDEX[] = {
    0, 4, 2, 2, 4, 6, // Front face
    1, 3, 5, 3, 7, 5, // Back face
    4, 5, 6, 6, 5, 7, // Left face
    0, 2, 1, 1, 2, 3, // Right face
    0, 1, 4, 4, 1, 5, // Top face
    2, 6, 3, 3, 6, 7 // Bottom face
};
// UV 坐标
inline Vec2 VERTICES_UV[] = {
    {1.0f, 1.0f}, {1.0f, 0.0f}, {0.0f, 1.0f}, {0.0f, 0.0f},
    {0.0f, 1.0f}, {0.0f, 0.0f}, {1.0f, 1.0f}, {1.0f, 0.0f}
};

struct Ray
{
    Vec3 origin;
    Vec3 direction;
    Vec3 get_point(float t) const
    {
        return origin + direction * t;
    }
};

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
    void reset();
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

enum GeometryType
{
    TRI,
    SPHERE,
    QUAD,
    GeometryCount,
};
int geometry_size(GeometryType type);
void build_geometry(GeometryType type,Geometry* geometry);

struct Geometry : std::enable_shared_from_this<Geometry>
{
    int id = 0;
    GeometryType geometry_type;
    explicit Geometry() = default;
    explicit Geometry(GeometryType sphere);
    virtual ~Geometry() = default;
    Box<3> box;
    virtual bool intersect(const L_MATH::Vec<float, 3>& point, const L_MATH::Vec<float, 3>& dir,
                           RayCasterResult* result) =0;
    virtual Vec3 get_normal(const L_MATH::Vec<float, 3>& point) =0;
    virtual Vec2 get_uv(const L_MATH::Vec<float, 3>& point) =0;
    virtual bool inside(const Vec3& point) =0;
    virtual float pdf(const Vec3& origin, const Vec3& dir) =0;
    virtual Vec3 random(const Vec3& origin) =0;
    virtual void transform(const Mat44& mat) =0;
    virtual void clone(Geometry* src) const =0;
}
;

class Sphere : public Geometry
{
public:
    explicit Sphere();
    Vec3 center;
    float radius;
    Sphere(const Vec3& center, float radius);
    L_MATH::Vec<float, 3> get_normal(const L_MATH::Vec<float, 3>& point) override;
    L_MATH::Vec<float, 2> get_uv(const L_MATH::Vec<float, 3>& point) override;
    bool inside(const L_MATH::Vec<float, 3>& point) override;
    bool intersect(const L_MATH::Vec<float, 3>& point, const L_MATH::Vec<float, 3>& dir,
                   RayCasterResult* result) override;
    float pdf(const L_MATH::Vec<float, 3>& origin, const L_MATH::Vec<float, 3>& dir) override;
    L_MATH::Vec<float, 3> random(const L_MATH::Vec<float, 3>& origin) override;
    void transform(const L_MATH::Mat<float, 4, 4>& mat) override;
    void clone(Geometry* src) const override;

};

class Quad : public Geometry
{
public:
    Vec3 p0, p1, p2;
    explicit Quad();
    Quad(const Vec3& p0, const Vec3& p1, const Vec3& p2);;
    L_MATH::Vec<float, 3> get_normal(const L_MATH::Vec<float, 3>& point) override;
    L_MATH::Vec<float, 2> get_uv(const L_MATH::Vec<float, 3>& point) override;
    bool inside(const L_MATH::Vec<float, 3>& point) override;
    bool intersect(const L_MATH::Vec<float, 3>& point, const L_MATH::Vec<float, 3>& dir,
                   RayCasterResult* result) override;
    float pdf(const L_MATH::Vec<float, 3>& origin, const L_MATH::Vec<float, 3>& dir) override;
    Vec3 random(const L_MATH::Vec<float, 3>& origin) override;
    void transform(const L_MATH::Mat<float, 4, 4>& mat) override;
    void clone(Geometry* src) const override;
};



#endif //Box_H
