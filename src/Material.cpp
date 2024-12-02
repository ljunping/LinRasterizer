//
// Created by Lin on 2024/12/4.
//

#include "Material.h"

void load_mtx_file(Material* material, const char* filename)
{
    FILE* file = fopen(filename, "r");
}

int MaterialManager::create_material()
{
    auto* material = new Material();
    material->id = material_id;
    return material_id++;
}

int MaterialManager::create_material( const char* mtx_file)
{
    auto* material = new Material();
    material->id = material_id;
    load_mtx_file(material, mtx_file);
    return material_id++;
}

void MaterialManager::delete_material(int id)
{
    auto* material = materials[id];
    delete material;
}

Material* MaterialManager::get_material(int id)
{
    return materials[id];
}
