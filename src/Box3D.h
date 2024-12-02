//
// Created by Lin on 2024/11/14.
//

#ifndef BOX3D_H
#define BOX3D_H
#include"L_math.h"
#include "PODPool.h"
#include <queue>

class TrianglePrimitive;

struct RayCasterResult
{
    TrianglePrimitive* triangle;
    Vec3 alpha;
    float t;
    float z;
};
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
    auto inside(const L_MATH::Vec<float, N>& point) const -> bool;
    bool intersect( const L_MATH::Vec<float, N>& origin,const L_MATH::Vec<float, N>& dir,  float& tMin, float& tMax) const;
};

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
auto Box<N>::inside(const L_MATH::Vec<float, N>& point) const -> bool
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

class Box3D : public Box<3>
{
public:
    Box3D() = default;

    explicit Box3D(const L_MATH::Vec<float, 3>& p): Box<3>(p)
    {
    };

    Box3D(const L_MATH::Vec<float, 3>& p, const L_MATH::Vec<float, 3>& q): Box<3>(p, q)
    {
    }
}
;

class BVHInterface
{
public:
    Box3D box;

    virtual bool intersect_3D(const L_MATH::Vec<float, 3>& point, const Vec3& dir, RayCasterResult* result)
    {
        return false;
    };

    virtual bool intersect_2D(const L_MATH::Vec<float, 3>& point, RayCasterResult* result)
    {
        return false;
    }

    virtual ~BVHInterface() = default;
};

struct BVHNode {
    Box3D aabb;
    BVHNode* left = nullptr;
    BVHNode* right = nullptr;
    BVHInterface* geometry = nullptr;
};


class BVHTree
{
    std::vector<BVHNode*> nodes;
    int node_index = 0;
    int block_size=0;
    void alloc_blocks(int count)
    {
        for (int i = 0; i < count; ++i)
        {
            auto var = (BVHNode*)malloc(block_size*sizeof(BVHNode));
            nodes.emplace_back(var);
        }
    }
    BVHNode* alloc_node()
    {
        while (node_index+1>=nodes.size()*block_size)
        {
            alloc_blocks(1);
        }
        int i = node_index / block_size;
        int j = node_index % block_size;
        ++node_index;
        return nodes[i] + j;
    }

    template <typename T>
    BVHNode* build_BVH(std::vector<T*>& geometries, int depth = 0)
    {
        if (geometries.size() == 1)
        {
            return new_BVHNode(geometries[0]->box, nullptr, nullptr, geometries[0]);
        }

        int axis = depth % 3;
        std::sort(geometries.begin(), geometries.end(), [axis](T* a, T* b)
        {
            return a->box.min[axis] < b->box.min[axis];
        });

        size_t mid = geometries.size() / 2;
        std::vector leftGeometries(geometries.begin(), geometries.begin() + mid);
        std::vector rightGeometries(geometries.begin() + mid, geometries.end());

        BVHNode* leftBVH = build_BVH(leftGeometries, depth + 1);
        BVHNode* rightBVH = build_BVH(rightGeometries, depth + 1);
        Box3D combinedAABB;
        combinedAABB.expand(leftBVH->aabb);
        combinedAABB.expand(rightBVH->aabb);
        return new_BVHNode(combinedAABB, leftBVH, rightBVH, nullptr);
    }


    bool traverse_BVH(const BVHNode* node, const Vec3& origin, const Vec3& dir, float tMin, float tMax,
                      std::vector<RayCasterResult>& result)
    {
        if (!node || !intersect_box(node->aabb, origin, dir, tMin, tMax))
        {
            return false;
        }
        if (node->geometry)
        {
            result.emplace_back();
            if (!node->geometry->intersect_3D(origin, dir, &result.back()))
            {
                result.pop_back();
                return false;
            }
            return true;
        }
        bool a = traverse_BVH(node->left, origin, dir, tMin, tMax, result);
        bool b = traverse_BVH(node->right, origin, dir, tMin, tMax, result);

        return a || b;
    }

    bool intersect_box(const Box3D& box, const Vec3& origin, const Vec3& dir, float& tMin, float& tMax)
    {
        for (int i = 0; i < 3; ++i)
        {
            if (!L_MATH::is_zero(dir[i]))
            {
                float invD = 1.0f / dir[i];
                float t0 = (box.min[i] - origin[i]) * invD;
                float t1 = (box.max[i] - origin[i]) * invD;
                if (invD < 0.0f) std::swap(t0, t1);
                tMin = t0 > tMin ? t0 : tMin;
                tMax = t1 < tMax ? t1 : tMax;
                if (tMax < tMin) return false;
            }
            else
            {
                if (origin[i] < box.min[i] || origin[i] > box.max[i])
                {
                    return false;
                }
            }
        }
        return tMax > 0;
    }

public:
    BVHNode* root{};

    BVHTree() : root(nullptr)
    {
    };

    template <typename T>
    explicit BVHTree(std::vector<T*>& geometries, int depth = 0)
    {
        build(geometries, depth);
    }

    template <typename T>
    void build(std::vector<T*>& geometries, int depth = 0)
    {
        static_assert(std::is_base_of<BVHInterface, T>::value, "T must inherit from BVHInterface");
        if (geometries.size() == 0)
        {
            return;
        }
        block_size = L_MATH::floor_pot(geometries.size());
        root = build_BVH(geometries, depth);
    }

    BVHNode* new_BVHNode(Box3D& box_3d, BVHNode* left, BVHNode* right, BVHInterface* tri)
    {
        auto bvh_node = alloc_node();
        bvh_node->aabb = box_3d;
        bvh_node->left = left;
        bvh_node->right = right;
        bvh_node->geometry = tri;
        return bvh_node;
    }

    void clear()
    {
        for (auto node : nodes)
        {
            free(node);
        }
        nodes.clear();
        node_index = 0;
    }

    bool intersect_traverse(const Vec3& origin, const Vec3& dir, std::vector<RayCasterResult>& result)
    {
        float minT = -INFINITY;
        float maxT = INFINITY;
        return traverse_BVH(root, origin, dir, minT, maxT, result);
    }

    bool intersect_compare_distance(const Vec3& origin, const Vec3& dir, RayCasterResult* result,
                                    float (*box_distance)(Box3D& box),
                                    float (*geometrie_distance)(RayCasterResult* result))
    {

        std::priority_queue<std::pair<float, BVHNode*>, std::vector<std::pair<float, BVHNode*>>, std::greater<>> queue;
        float minT = -INFINITY, max = INFINITY;
        if (intersect_box(root->aabb, origin, dir, minT, max))
        {
            queue.emplace(box_distance(root->aabb), root);
        }
        float maxDistance = -std::numeric_limits<float>::infinity();
        bool find = false;
        while (!queue.empty())
        {
            auto& top = queue.top();
            auto distance = top.first;
            auto node = top.second;
            queue.pop();
            if (distance <= maxDistance)
            {
                break;
            }
            if (node->geometry && node->geometry->intersect_3D(origin, dir, result))
            {
                auto dist = geometrie_distance(result);
                if (dist > maxDistance)
                {
                    maxDistance = dist;
                    find = true;
                }
            }
            else
            {
                if (node->left)
                {
                    if (node->left->aabb.intersect(origin, dir, minT, max))
                    {
                        queue.emplace(box_distance(node->left->aabb), node->left);
                    }
                }
                if (node->right)
                {
                    if (node->right->aabb.intersect(origin, dir, minT, max))
                    {
                        queue.emplace(box_distance(node->right->aabb), node->right);
                    }
                }
            }
        }
        return find;
    }
};

#endif //BOX3D_H
