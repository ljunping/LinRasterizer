#include "Box.h"

#include "TrianglePrimitive.h"


static int intersect_traverse_count = 0;


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


void BVHTree::build(std::vector<TrianglePrimitive*>& geometries, int depth)
{
    if (geometries.size() == 0)
    {
        return;
    }
    clear();
    block_size = L_MATH::floor_pot(geometries.size());
    root = build_BVH(geometries.begin(), geometries.end(), depth);
}

void BVHTree::alloc_blocks(int count)
{
    for (int i = 0; i < count; ++i)
    {
        auto var = (BVHNode*)malloc(block_size*sizeof(BVHNode));
        nodes.emplace_back(var);
    }
}

BVHNode* BVHTree::alloc_node()
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

BVHNode* BVHTree::build_BVH(std::vector<TrianglePrimitive*>::iterator begin,
                            std::vector<TrianglePrimitive*>::iterator end, int depth)
{
    int size = end - begin;
    if (size == 1)
    {
        return new_BVHNode((*begin)->box, nullptr, nullptr, *begin);
    }
    int axis = depth % 3;
    parallel_sort(begin, end, [axis](TrianglePrimitive* a, TrianglePrimitive* b)
    {
        return a->box.min[axis] < b->box.min[axis];
    });
    size_t mid = size / 2;
    BVHNode* leftBVH = build_BVH(begin, begin + mid, depth + 1);
    BVHNode* rightBVH = build_BVH(begin + mid, end, depth + 1);
    Box<3> combinedAABB;
    combinedAABB.expand(leftBVH->aabb);
    combinedAABB.expand(rightBVH->aabb);
    return new_BVHNode(combinedAABB, leftBVH, rightBVH, nullptr);
}


bool BVHTree::traverse_BVH(const BVHNode* node, const L_MATH::Vec<float, 3>& origin,
                                  const L_MATH::Vec<float, 3>& dir, float tMin, float tMax, std::vector<RayCasterResult>& result)
{
    // intersect_traverse_count++;
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
bool BVHTree::intersect_box(const Box<3>& box, const L_MATH::Vec<float, 3>& origin,
    const L_MATH::Vec<float, 3>& dir, float& tMin, float& tMax)
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

inline BVHNode* BVHTree::new_BVHNode(Box<3>& box_3d, BVHNode* left, BVHNode* right, TrianglePrimitive* tri)
{
    auto bvh_node = alloc_node();
    bvh_node->aabb = box_3d;
    bvh_node->left = left;
    bvh_node->right = right;
    bvh_node->geometry = tri;
    return bvh_node;
}

inline void BVHTree::clear()
{
    for (auto node : nodes)
    {
        free(node);
    }
    nodes.clear();
    node_index = 0;
}

bool BVHTree::intersect_traverse(const L_MATH::Vec<float, 3>& origin, const L_MATH::Vec<float, 3>& dir,
                                        std::vector<RayCasterResult>& result)
{
    float minT = -INFINITY;
    float maxT = INFINITY;
    // intersect_traverse_count = 0;
    bool res = traverse_BVH(root, origin, dir, minT, maxT, result);
    // printf("intersect_traverse %d\n", intersect_traverse_count);
    return res;
}

bool BVHTree::intersect_compare_distance(const L_MATH::Vec<float, 3>& origin, const L_MATH::Vec<float, 3>& dir,
    RayCasterResult* result, float(* box_distance)(Box<3>& box), float(* geometrie_distance)(RayCasterResult* result))
{

    std::priority_queue<std::pair<float, BVHNode*>, std::vector<std::pair<float, BVHNode*>>, std::greater<>> queue;
    float minT = -std::numeric_limits<float>::infinity(), max = std::numeric_limits<float>::infinity();
    if (intersect_box(root->aabb, origin, dir, minT, max))
    {
        queue.emplace(box_distance(root->aabb), root);
    }
    float maxDistance = -std::numeric_limits<float>::infinity();
    bool find = false;
    // int __count = 0;
    while (!queue.empty())
    {
        // __count++;
        auto top = queue.top();
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
            if (node->left && node->left->aabb.intersect(origin, dir, minT, max))
            {
                queue.emplace(box_distance(node->left->aabb), node->left);
            }
            if (node->right && node->right->aabb.intersect(origin, dir, minT, max))
            {
                queue.emplace(box_distance(node->right->aabb), node->right);
            }
        }
    }
    // printf("intersect_compare_distance %d\n", __count);
    return find;
}