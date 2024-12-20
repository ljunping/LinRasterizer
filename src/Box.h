//
// Created by Lin on 2024/11/14.
//

#ifndef Box_H
#define Box_H
#include"L_math.h"
#include <queue>

#include "JobSystem.h"

class TrianglePrimitive;

struct RayCasterResult
{
    TrianglePrimitive* triangle;
    Vec3 alpha;
    float t;
};
template<int N>
class alignas(16) Box;

template<int N>
class alignas(16) Box
{
public:
    L_MATH::Vec<float, N> min;
    L_MATH::Vec<float, N> max;
    Box();
    explicit Box(const L_MATH::Vec<float, N>& p);

    Box(const L_MATH::Vec<float, N>& p, const L_MATH::Vec<float, N>& q);

    void expand(const L_MATH::Vec<float, N>& v);
    void expand(const Box<N>& v);
    template <int M>
    bool inside(const L_MATH::Vec<float, N>& point) const;
    bool intersect( const L_MATH::Vec<float, N>& origin,const L_MATH::Vec<float, N>& dir,  float& tMin, float& tMax) const;
};




struct BVHNode {
    Box<3> aabb;
    BVHNode* left = nullptr;
    BVHNode* right = nullptr;
    TrianglePrimitive* geometry = nullptr;
};


class BVHTree
{
    std::vector<BVHNode*> nodes;
    int node_index = 0;
    int block_size=0;
    void alloc_blocks(int count);

    BVHNode* alloc_node();

    BVHNode* build_BVH(typename std::vector<TrianglePrimitive*>::iterator begin, typename std::vector<TrianglePrimitive*>::iterator end, int depth = 0);


    bool traverse_BVH(const BVHNode* node, const Vec3& origin, const Vec3& dir, float tMin, float tMax,
                      std::vector<RayCasterResult>& result);

    static bool intersect_box(const Box<3>& box, const Vec3& origin, const Vec3& dir, float& tMin, float& tMax);

public:
    BVHNode* root{};

    BVHTree() : root(nullptr)
    {
    };
    void build(std::vector<TrianglePrimitive*>& geometries, int depth = 0);

    BVHNode* new_BVHNode(Box<3>& box_3d, BVHNode* left, BVHNode* right, TrianglePrimitive* tri);

    void clear();

    bool intersect_traverse(const Vec3& origin, const Vec3& dir, std::vector<RayCasterResult>& result);

    bool intersect_compare_distance(const Vec3& origin, const Vec3& dir, RayCasterResult* result,
                                    float (*box_distance)(Box<3>& box),
                                    float (*geometrie_distance)(RayCasterResult* result));
};

#endif //Box_H
