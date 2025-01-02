//
// Created by Lin on 2024/11/15.
//

#include "Mesh.h"

#include <iostream>
#include <stdexcept>

#include "debug.h"
#include "PODPool.h"
#include "TrianglePrimitive.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "CommonMacro.h"

int Mesh::_id = 0;

Mesh::~Mesh()
{

}

Mesh::Mesh(std::shared_ptr<float[]>& vbo, int count): vbo(vbo), data_size(count)
{
}
int _VALUE=0;
//只加载第一个mesh,一般只有一个
Mesh::Mesh(const char* obj_file_name)
{
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(obj_file_name, aiProcess_Triangulate | aiProcess_FlipUVs);
    if (!scene || !scene->mRootNode)
    {
        std::cerr << "Failed to load model: " << importer.GetErrorString() << std::endl;
        return;
    }
    for (int index = 0; index < scene->mNumMeshes; ++index)
    {
        std::vector<float> data;
        std::vector<int> ebo;
        std::vector<AttributeDataFormat> attribute_data_formats;
        // 递归遍历模型的所有网格
        aiMesh* mesh = scene->mMeshes[index]; // 假设只有一个网格
        int offset = 0;
        for (unsigned int i = 0; i < mesh->mNumVertices; ++i)
        {
            if (i == 0)
            {
                offset = 0;
            }
            data.push_back(mesh->mVertices[i].x);
            data.push_back(mesh->mVertices[i].y);
            data.push_back(mesh->mVertices[i].z);
            if (i == 0)
            {
                attribute_data_formats.emplace_back(POS, 3, offset, 0);
                offset = offset + 3;
            }
            if (mesh->mNormals)
            {
                data.push_back(mesh->mNormals[i].x);
                data.push_back(mesh->mNormals[i].y);
                data.push_back(mesh->mNormals[i].z);
                if (i == 0)
                {
                    attribute_data_formats.emplace_back(NORMAL, 3, offset, 0);
                    offset = offset + 3;
                }
            }
            for (int j = 0; j < AI_MAX_NUMBER_OF_COLOR_SETS; ++j)
            {
                if (mesh->mColors[j])
                {
                    data.push_back(mesh->mColors[j][i].r);
                    data.push_back(mesh->mColors[j][i].g);
                    data.push_back(mesh->mColors[j][i].b);
                    data.push_back(mesh->mColors[j][i].a);
                    if (i == 0)
                    {
                        attribute_data_formats.emplace_back(COLOR, 4, offset, 0);
                        offset = offset + 4;
                    }
                }
            }
            for (int j = 0; j < AI_MAX_NUMBER_OF_TEXTURECOORDS; ++j)
            {
                if (mesh->mTextureCoords[j])
                {
                    data.push_back(mesh->mTextureCoords[j][i].x);
                    data.push_back(mesh->mTextureCoords[j][i].y);
                    if (i == 0)
                    {
                        attribute_data_formats.emplace_back(UV, 2, offset, 0);
                        offset = offset + 2;
                    }
                }
            }
            if (mesh->mTangents)
            {
                data.push_back(mesh->mTangents[i].x);
                data.push_back(mesh->mTangents[i].y);
                data.push_back(mesh->mTangents[i].z);
                if (i == 0)
                {
                    attribute_data_formats.emplace_back(TANGENT, 3, offset, 0);
                    offset = offset + 3;
                }
            }
        }

        // 处理索引
        for (unsigned int i = 0; i < mesh->mNumFaces; ++i)
        {
            const aiFace& face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; ++j)
            {
                ebo.push_back(face.mIndices[j]);
            }
        }
        std::shared_ptr<float[]> data_p(new float[data.size()]);

        for (int i = 0; i < data.size(); i++)
        {
            data_p[i] = data[i];
        }
        this->vbo = data_p;
        this->data_size = data.size();
        this->vert_data_length = offset;
        this->vert_count = data.size() / offset;
        this->ebo = std::move(ebo);

        for (auto& attribute_data_format : attribute_data_formats)
        {
            this->bind_attribute(attribute_data_format.type, attribute_data_format.attr_unit_size,
                                     attribute_data_format.offset, offset);

        }

        this->locate_zero();
        calculate_tangents();
        break;

    }

}

void Mesh::on_create()
{
    this->id = ++_id;
}
void Mesh::locate_zero()
{
    auto mesh_centroid = this->get_mesh_centroid();
    auto _offset = data_formats[POS].offset;
    for (int i = 0; i < vert_count; ++i)
    {

        (vbo.get() + i * vert_data_length)[_offset] -= mesh_centroid[0];
        (vbo.get() + i * vert_data_length)[_offset + 1] -= mesh_centroid[1];
        (vbo.get() + i * vert_data_length)[_offset + 2] -= mesh_centroid[2];
    }
    is_generate_box = false;
}


void Mesh::bind_attribute(AttributeType type, int attr_unit_size, int offset, int stride)
{
    data_formats[type] = {type, attr_unit_size, offset, stride};
    vert_data_length = stride;
    vert_count = data_size / vert_data_length;
}




void Mesh::generate_triangle_index(TrianglePrimitive& tri, int tri_index)
{
    tri.id = tri_index;
    tri.mesh = this;
    if (this->ebo.empty())
    {
        tri.vert_index[0] = tri_index * 3;
        tri.vert_index[1] = tri_index * 3 + 1;
        tri.vert_index[2] = tri_index * 3 + 2;
    }else
    {
        tri.vert_index[0] = ebo[tri_index * 3];
        tri.vert_index[1] = ebo[tri_index * 3 + 1];
        tri.vert_index[2] = ebo[tri_index * 3 + 2];
    }
}

void Mesh::generate_triangle(TrianglePrimitive& tri, int tri_index)
{
    this->generate_triangle_index(tri, tri_index);
    this->get_attribute_value(tri.vert_index[0], POS, tri.v[0]);
    this->get_attribute_value(tri.vert_index[1], POS, tri.v[1]);
    this->get_attribute_value(tri.vert_index[2], POS, tri.v[2]);
}

int Mesh::tri_count() const
{
    if (this->ebo.empty())
    {
        return vert_count / 3;
    }
    return ebo.size() / 3;
}


float* Mesh::operator[](int vert_index) const
{
    return vbo.get() + vert_index * vert_data_length;
}

L_MATH::Vec<float, 3> Mesh::get_mesh_centroid()
{
    return this->get_box().center();
}

void Mesh::calculate_tangents()
{
    if ((this->data_formats[UV].type != UV))
    {
        return;
    }
    if (this->data_formats[TANGENT].type == TANGENT)
    {
        return;
    }
    if (tangents.size() == vert_count)
    {
        return;
    }
    tangents.resize(vert_count);
    std::vector<int> tangents_count(vert_count, 0);
    for (int i = 0; i <tri_count(); ++i)
    {
        TrianglePrimitive triangle;
        generate_triangle_index(triangle, i);

        Vec2 uv0, uv1, uv2;
        get_attribute_value(triangle.vert_index[0], UV, uv0);
        get_attribute_value(triangle.vert_index[1], UV, uv1);
        get_attribute_value(triangle.vert_index[2], UV, uv2);

        Vec3 v0, v1, v2;
        get_attribute_value(triangle.vert_index[0], POS, v0);
        get_attribute_value(triangle.vert_index[1], POS, v1);
        get_attribute_value(triangle.vert_index[2], POS, v2);

        auto p1 = v1 - v0;
        auto p2 = v2 - v0;

        auto uv10 = uv1 - uv0;
        auto uv20 = uv2 - uv0;
        auto inv_f = uv10[0] * uv20[1] - uv10[1] * uv20[0];
        if (L_MATH::is_zero(inv_f))
        {
            continue;
        }
        float f = 1 / inv_f;
        auto tan = ((p1 * uv20[1] - p2 * uv10[1]) * f);
        tangents[triangle.vert_index[0]] += tan;
        tangents[triangle.vert_index[1]] += tan;
        tangents[triangle.vert_index[2]] += tan;
        ++tangents_count[triangle.vert_index[0]];
        ++tangents_count[triangle.vert_index[1]];
        ++tangents_count[triangle.vert_index[2]];
    }

    for (int i = 0; i < tangents_count.size(); ++i)
    {
        Vec3 normal;
        this->get_attribute_value(i, NORMAL, normal);
        if (tangents_count[i] == 0)
        {
            // 选择一个基向量，通常选择不平行于法线的向量
            Vec3 basis = (std::abs(normal[0]) < std::abs(normal[1])) ? Vec3{1, 0, 0} : Vec3{0, 1, 0};
            // 计算垂直于法线的向量
            tangents[i] = cross(normal, basis).normalize();
        }else
        {
            tangents[i] /= tangents_count[i];
            tangents[i].normalized();
            tangents[i] -= normal * dot(normal, tangents[i]);
            tangents[i].normalized();
        }
    }
}


Box<3> Mesh::get_box()
{
    if (is_generate_box)
    {
        return this->box;
    }
    Box<3> box;
    for (int i = 0; i < vert_count; ++i)
    {
        Vec3 pos;
        this->get_attribute_value(i, POS, pos);
        box.expand(pos);
    }
    this->box = box;
    this->is_generate_box = true;
    return box;
}




