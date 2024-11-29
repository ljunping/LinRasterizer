//
// Created by Lin on 2024/11/15.
//

#ifndef ATTRIBUTE_H
#define ATTRIBUTE_H
#pragma once
#include "L_math.h"

class TrianglePrimitive;
class Attributes;
enum AttributeType
{
    POS,
    NORMAL,
    COLOR,
    UV,
    TANGENT
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
    const Attributes* attributes;
    std::vector<float> values;
    int v[3];
    int data_count = 0;
    Vec3 alpha;
    template<int N>
    void get_attribute_value(int attribute_index, L_MATH::Vec<float, N>& result);
    void calculate_values();
    void reset();
};

class Attributes
{
public:
    static bool load_obj_file(const char *obj_file_name, std::vector<Attributes> &attributeses);

    std::vector<int> ebo;
    std::vector<AttributeDataFormat> data_formats;
    std::vector<int> textures;
    float* vbo;
    int data_size;
    int vert_count;
    int vert_data_length;
    ~Attributes();
    Attributes() = default;
    void bind_texture(int texture_id);
    Attributes(float *vbo, int count);
    int bind_attribute(AttributeType type, int attr_unit_size, int offset, int stride);
    void create_vert_attribute(int v0, int v1, int v2, const L_MATH::Vec<float, 3>& alpha, VertexAttribute& result) const;
    void create_vert_attribute(const VertexAttribute& v0, const VertexAttribute& v1, const VertexAttribute& v2,
                               const L_MATH::Vec<float, 3>& alpha, VertexAttribute& result) const;
    void generate_triangles(std::vector<TrianglePrimitive>& result) const;
    void generate_triangle(int v0, int v1, int v2, TrianglePrimitive& result) const;
    void generate_triangles(const std::vector<int>& ebo, std::vector<TrianglePrimitive>& result) const;
    float* operator[](int vert_index) const;
    template<int N>
    void get_attribute_value(int vert_index,int attribute_index, L_MATH::Vec<float, N>& result) const;
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
void Attributes::get_attribute_value(int vert_index, int attribute_index, L_MATH::Vec<float, N>& result) const
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
