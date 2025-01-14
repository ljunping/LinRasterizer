//
// Created by Lin on 2025/1/1.
//

#ifndef BVHTREE_H
#define BVHTREE_H
#include <vector>
#include "DrawCallContext.h"
#include "Geometry.h"
#include "L_math.h"

struct RayCasterResult;
class DrawCallContext;
struct TrianglePrimitive;



struct TrianglePrimitive;


struct RayCasterResult
{
    Geometry* geometry;
    TrianglePrimitive* triangle;
    Vec3 alpha;
    float t;
};

struct BVHNode
{
    Box<3> aabb;
    BVHNode* left = nullptr;
    BVHNode* right = nullptr;
    Geometry* geometry = nullptr;
};


class BVHTree
{
    std::vector<BVHNode*> nodes;
    int node_index = 0;
    int block_size=0;
    void alloc_blocks(int count);

    BVHNode* alloc_node();

    BVHNode* build_BVH(typename std::vector<Geometry*>::iterator begin, typename std::vector<Geometry*>::iterator end, int depth = 0);


    bool traverse_BVH(const BVHNode* node, const L_MATH::Vec<float, 3>& origin, const L_MATH::Vec<float, 3>& dir, float tMin, float tMax,
                      std::vector<RayCasterResult>& result, bool (*condition)(Geometry*, void*), void* data);

    static bool intersect_box(const Box<3>& box, const Vec3& origin, const Vec3& dir, float& tMin, float& tMax);

public:
    BVHNode* root{};

    BVHTree() : root(nullptr)
    {
    };
    void build(std::vector<Geometry*>& geometries, int depth = 0);

    BVHNode* new_BVHNode(Box<3>& box_3d, BVHNode* left, BVHNode* right, Geometry* tri);

    void clear();

    bool intersect_nearest(const L_MATH::Vec<float, 3>& origin, const L_MATH::Vec<float, 3>& dir,
                           RayCasterResult& result, bool (*
                               condition)(Geometry*, void*), void* data);



    bool intersect_traverse(const L_MATH::Vec<float, 3>& origin, const L_MATH::Vec<float, 3>& dir, std::vector<RayCasterResult>& result, bool (*
                                condition)(Geometry*, void*), void* data);

    bool intersect_compare_distance(const Vec3& origin, const Vec3& dir, RayCasterResult* result,
                                    float (*box_distance)(Box<3>& box),
                                    float (*geometrie_distance)(RayCasterResult* result));
};




#endif //BVHTREE_H
