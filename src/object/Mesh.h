//
// Created by Lin on 2024/11/15.
//

#ifndef ATTRIBUTE_H
#define ATTRIBUTE_H
#pragma once
#include "Box.h"
#include "L_math.h"
#include "Object.h"
#include "Resource.h"

struct VertexInterpolation;
class Mesh;
class TrianglePrimitive;

struct AttributeDataFormat
{
    AttributeType type;
    int attr_unit_size;
    int offset;
    int stride;
};


class Mesh : public Resource
{
    static int _id;
    INIT_TYPE(Mesh, Resource)
    Mesh(SHARE_PTR<float[]>& vbo, int count);
    explicit Mesh(const char* obj_file_name);
    Vec3 centroid;
    Box<3> box;
    bool is_centroid = false;
    bool is_generate_box = false;
public:
    int id = 0;
    void on_create() override;
    std::vector<int> ebo;
    AttributeDataFormat data_formats[AttributeTypeCount]{};
    SHARE_PTR<float[]> vbo;
    int data_size;
    int vert_count;
    int vert_data_length;
    std::vector<Vec3> tangents;
    ~Mesh();
    void bind_attribute(AttributeType type, int attr_unit_size, int offset, int stride);
    void generate_triangle_index(TrianglePrimitive& tri, int tri_index);
    void generate_triangle(TrianglePrimitive& tri, int tri_index);
    int tri_count() const;
    float* operator[](int vert_index) const;
    template <int N>
    void get_attribute_value(int vert_index, int attribute_index, L_MATH::Vec<float, N>& result) const;
    Vec3 get_mesh_centroid();
    void calculate_tangents();
    Box<3> get_box();
}
;





template <int N>
void Mesh::get_attribute_value(int vert_index, int attribute_index, L_MATH::Vec<float, N>& result) const
{
    if constexpr (N == 3)
    {
        if (attribute_index == TANGENT && data_formats[attribute_index].type != TANGENT && data_formats[UV].type == UV)
        {
            result = tangents[vert_index];
            return;
        }
    }

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
