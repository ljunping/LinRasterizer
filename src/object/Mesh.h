//
// Created by Lin on 2024/11/15.
//

#ifndef ATTRIBUTE_H
#define ATTRIBUTE_H
#pragma once
#include "CommonMacro.h"
#include "L_math.h"
#include "Object.h"
#include "Resource.h"

class Mesh;
class TrianglePrimitive;
enum AttributeType
{
    INVALID,
    POS,
    NORMAL,
    COLOR,
    UV,
    TANGENT,
    AttributeTypeCount,
};
struct AttributeDataFormat
{
    AttributeType type;
    int attr_unit_size;
    int offset;
    int stride;
};

class VertexAttribute
{
public:
    const Mesh* attributes;
    std::vector<float> values;
    int v[3];
    int data_count = 0;
    Vec3 alpha;
    template<int N>
    void get_attribute_value(int attribute_index, L_MATH::Vec<float, N>& result);
    void calculate_values();
    void reset();
};

class Mesh : public Resource
{
    static int _id;
    INIT_TYPE(Mesh, Resource)
    Mesh(SHARE_PTR<float[]>& vbo, int count);
    explicit Mesh(const char* obj_file_name);
public:
    int id = 0;
    void on_create() override;
    std::vector<int> ebo;
    AttributeDataFormat data_formats[AttributeTypeCount];
    SHARE_PTR<float[]> vbo;
    int data_size;
    int vert_count;
    int vert_data_length;
    ~Mesh();
    void bind_attribute(AttributeType type, int attr_unit_size, int offset, int stride);
    void create_vert_attribute(int v0, int v1, int v2, const L_MATH::Vec<float, 3>& alpha,
                               VertexAttribute& result) const;
    void create_vert_attribute(const VertexAttribute& v0, const VertexAttribute& v1, const VertexAttribute& v2,
                               const L_MATH::Vec<float, 3>& alpha, VertexAttribute& result) const;
    void generate_triangles(std::vector<TrianglePrimitive>& result) const;
    void generate_triangle(TrianglePrimitive& tri, int tri_index) const;
    int tri_count() const;
    void generate_triangle(int v0, int v1, int v2, TrianglePrimitive& result) const;
    void generate_triangles(const std::vector<int>& ebo, std::vector<TrianglePrimitive>& result) const;
    float* operator[](int vert_index) const;
    template <int N>
    void get_attribute_value(int vert_index, int attribute_index, L_MATH::Vec<float, N>& result) const;
};


template <int N>
void VertexAttribute::get_attribute_value(int attribute_index, L_MATH::Vec<float, N>& result)
{
    auto offset = attributes->data_formats[attribute_index].offset;
    if constexpr (N == 3)
    {
        result[0] = values[offset];
        result[1] = values[offset + 1];
        result[2] = values[offset + 2];
    }

    if constexpr (N == 2)
    {
        result[0] = values[offset];
        result[1] = values[offset + 1];
    }

    if constexpr (N == 4)
    {
        result[0] = values[offset];
        result[1] = values[offset + 1];
        result[2] = values[offset + 2];
        result[3] = values[offset + 3];
    }
}

template <int N>
void Mesh::get_attribute_value(int vert_index, int attribute_index, L_MATH::Vec<float, N>& result) const
{
    auto offset = data_formats[attribute_index].offset;
    auto values = (*this)[vert_index];
    if constexpr (N == 3)
    {
        result[0] = values[offset];
        result[1] = values[offset + 1];
        result[2] = values[offset + 2];
    }

    if constexpr (N == 2)
    {
        result[0] = values[offset];
        result[1] = values[offset + 1];
    }

    if constexpr (N == 4)
    {
        result[0] = values[offset];
        result[1] = values[offset + 1];
        result[2] = values[offset + 2];
        result[3] = values[offset + 3];
    }
}




#endif //ATTRIBUTE_H
