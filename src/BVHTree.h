//
// Created by Lin on 2025/1/1.
//

#ifndef BVHTREE_H
#define BVHTREE_H
#include <vector>

#include "Geometry.h"
#include "L_math.h"

struct TrianglePrimitive;

struct RayCasterResult
{
    TrianglePrimitive* triangle;
    Vec3 alpha;
    float t;
};

struct TrianglePrimitive;

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




#endif //BVHTREE_H
