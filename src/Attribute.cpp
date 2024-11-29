//
// Created by Lin on 2024/11/15.
//

#include "Attribute.h"

#include <stdexcept>

#include "debug.h"
#include "PODPool.h"
#include "TrianglePrimitive.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

Attributes::~Attributes()
{

}

void Attributes::bind_texture(int i)
{
    textures.push_back(i);
}

Attributes::Attributes(float *data, int count):vbo(data), data_size(count)
{
}


bool Attributes::load_obj_file(const char *obj_file_name, std::vector<Attributes> &attributeses)
{
    Assimp::Importer importer;
    const aiScene *scene = importer.ReadFile(obj_file_name, aiProcess_Triangulate | aiProcess_FlipUVs);
    if (!scene || !scene->mRootNode)
    {
        std::cerr << "Failed to load model: " << importer.GetErrorString() << std::endl;
        return false;
    }
    attributeses.resize(scene->mNumMeshes);
    for (int index = 0; index < scene->mNumMeshes; ++index)
    {
        std::vector<float> data;
        std::vector<int> ebo;
        std::vector<AttributeDataFormat> attribute_data_formats;
        // 递归遍历模型的所有网格
        aiMesh *mesh = scene->mMeshes[index]; // 假设只有一个网格
        int offset = 0;
        for (unsigned int i = 0; i < mesh->mNumVertices; ++i)
        {

            data.push_back(mesh->mVertices[i].x);
            data.push_back(mesh->mVertices[i].y);
            data.push_back(mesh->mVertices[i].z);
            attribute_data_formats.emplace_back(POS, 3, 0, 0);
            offset = offset + 3;
            if (mesh->mNormals)
            {
                data.push_back(mesh->mNormals[i].x);
                data.push_back(mesh->mNormals[i].y);
                data.push_back(mesh->mNormals[i].z);
                attribute_data_formats.emplace_back(NORMAL, 3, offset, 0);
                offset = offset + 3;
            }
            for (int j = 0; j < AI_MAX_NUMBER_OF_COLOR_SETS; ++j)
            {
                if (mesh->mColors[j])
                {
                    data.push_back(mesh->mColors[j][i].r);
                    data.push_back(mesh->mColors[j][i].g);
                    data.push_back(mesh->mColors[j][i].b);
                    data.push_back(mesh->mColors[j][i].a);
                    attribute_data_formats.emplace_back(COLOR, 4, offset, 0);
                    offset = offset + 4;
                }
            }
            for (int j = 0; j < AI_MAX_NUMBER_OF_TEXTURECOORDS; ++j)
            {
                if (mesh->mTextureCoords[j])
                {
                    data.push_back(mesh->mTextureCoords[j][i].x);
                    data.push_back(mesh->mTextureCoords[j][i].y);
                    attribute_data_formats.emplace_back(UV, 2, offset, 0);
                    offset = offset + 2;
                }
            }
            if (mesh->mTangents)
            {
                data.push_back(mesh->mTangents[i].x);
                data.push_back(mesh->mTangents[i].y);
                data.push_back(mesh->mTangents[i].z);
                attribute_data_formats.emplace_back(TANGENT, 3, offset, 0);
                offset = offset + 3;
            }
        }

        // 处理索引
        for (unsigned int i = 0; i < mesh->mNumFaces; ++i)
        {
            const aiFace &face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; ++j)
            {
                ebo.push_back(face.mIndices[j]);
            }
        }

        float *data_p = new float[data.size()];
        for (int i = 0; i < data.size(); i++)
        {
            data_p[i] = data[i];
        }
        attributeses[index].vbo = data_p;
        attributeses[index].data_size = data.size();
        attributeses[index].vert_data_length = offset;
        attributeses[index].vert_count = data.size() / offset;
        attributeses[index].ebo = std::move(ebo);
        attributeses[index].data_formats = std::move(attribute_data_formats);
    }
    return true;
}



int Attributes::bind_attribute(AttributeType type,int attr_unit_size, int offset, int stride)
{
    data_formats.emplace_back(type,attr_unit_size, offset, stride);
    vert_data_length = stride;
    vert_count = data_size / vert_data_length;
    return data_formats.size() - 1;
}

void Attributes::create_vert_attribute(int v0, int v1, int v2, const L_MATH::Vec<float, 3>& alpha,
    VertexAttribute& result) const
{
    result.v[0] = v0;
    result.v[1] = v1;
    result.v[2] = v2;
    result.alpha = alpha;
    result.attributes = this;
    result.calculate_values();
}

void Attributes::create_vert_attribute(const VertexAttribute& v0, const VertexAttribute& v1, const VertexAttribute& v2,
    const L_MATH::Vec<float, 3>& alpha, VertexAttribute& result) const
{
    auto vec = v0.alpha * alpha[0] + v1.alpha * alpha[1] + v2.alpha * alpha[2];
    create_vert_attribute(v0.v[0], v0.v[1], v0.v[2], vec, result);
}

void Attributes::generate_triangles(std::vector<TrianglePrimitive>& result) const
{
    if (this->ebo.empty())
    {
        for (int i = 0; i < vert_count; i = i + 3)
        {
            auto& triangle_primitive = result.emplace_back();
#if DEBUG
            triangle_primitive.id = i/3;
#endif
            generate_triangle(i, i + 1, i + 2, triangle_primitive);
        }
    }else
    {
        this->generate_triangles(this->ebo, result);
    }

}

void Attributes::generate_triangle(int v0, int v1, int v2,TrianglePrimitive& result) const
{
    create_vert_attribute(v0, v1, v2,Vec3{1, 0, 0}, result.vert[0]);
    create_vert_attribute(v0, v1, v2,Vec3{0, 1, 0}, result.vert[1]);
    create_vert_attribute(v0, v1, v2,Vec3{0, 0, 1}, result.vert[2]);
    result.vert[0].get_attribute_value(0, result.v[0]);
    result.vert[1].get_attribute_value(0, result.v[1]);
    result.vert[2].get_attribute_value(0, result.v[2]);
    result.inv_w = Vec3::ONE;
}

void Attributes::generate_triangles(const std::vector<int>& ebo, std::vector<TrianglePrimitive>& result) const
{
    for (int i = 0; i < ebo.size(); i = i + 3)
    {
        auto& triangle_primitive = result.emplace_back();
#if DEBUG
        triangle_primitive.id = i/3;
#endif
        generate_triangle(ebo[i], ebo[i+1], ebo[i+2], triangle_primitive);
    }
}

float* Attributes::operator[](int vert_index) const
{
    return vbo + vert_index * vert_data_length;
}


void VertexAttribute::calculate_values()
{
    if (values.size()!=attributes->vert_data_length)
    {
        values.resize(attributes->vert_data_length);
    }
    for (int i = 0; i < attributes->vert_data_length; ++i)
    {
        values[i] = (*attributes)[v[0]][i] * alpha[0] + (*attributes)[v[1]][i] * alpha[1] + (*attributes)[v[2]][i] *
            alpha[2];
    }
}

void VertexAttribute::reset()
{
}

