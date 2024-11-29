//
// Created by Lin on 2024/11/15.
//

#include "Scene.h"
#include "TrianglePrimitive.h"


int Scene::next_triangle_id()
{
    return current_triangle_id++;
}

Scene::Scene()
{
    current_triangle_id = 0;
    model_matrix = L_MATH::Mat<float, 4, 4>::IDENTITY;
}

Scene::~Scene()
{
    clear();
}

void Scene::update_model_matrix(const L_MATH::Mat<float, 4, 4>& mat)
{
    model_matrix = mat;
}

int Scene::add_origin_triangle(TrianglePrimitive& tri)
{
    auto tri_ = new TrianglePrimitive();
    *tri_ = tri;
    int triangle_id = next_triangle_id();
    scene_triangle_map[triangle_id] = tri_;
    model_triangles.push_back(tri_);
    return triangle_id;
}

int Scene::add_origin_triangles(std::vector<TrianglePrimitive*>& tris)
{
    for (auto tri : tris)
    {
        this->add_origin_triangle(*tri);
    }
}

int Scene::add_attributes(Attributes *attrs)
{
    std::vector<TrianglePrimitive> tris;
    attrs->generate_triangles(tris);
    this->add_origin_triangles(tris);
}

int Scene::add_origin_triangles(std::vector<TrianglePrimitive>& tris)
{
    for (auto& tri : tris)
    {
        this->add_origin_triangle(tri);
    }
}

void Scene::remove_origin_triangle(int id)
{
    if (scene_triangle_map.contains(id))
    {
        auto scene_item = scene_triangle_map[id];
        scene_triangle_map.erase(id);
        model_triangles.easy_remove(scene_item);
        delete scene_item;
    }
}

void Scene::clear()
{
    current_triangle_id = 0;
    for (auto model_triangle : model_triangles)
    {
        delete model_triangle;
    }
    model_triangles.clear();
    model_matrix = L_MATH::Mat<float, 4, 4>::IDENTITY;
}

void Scene::update_origin_triangle(int id,  TrianglePrimitive* tri)
{
    if (scene_triangle_map.contains(id))
    {
        auto& scene_item = scene_triangle_map[id];
        *scene_item = *tri;
    }
}

VectorRemoveEasy<TrianglePrimitive*>& Scene::get_model_triangle_list()
{
    return model_triangles;
}

int Scene::get_triangle_count() const
{
    return model_triangles.valid_size();
}

