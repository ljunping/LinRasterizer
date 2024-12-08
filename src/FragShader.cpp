//
// Created by Lin on 2024/11/16.
//

#include "FragShader.h"

#include "Material.h"

void FragShader::init()
{
    material = MATERIAL_MANAGER.get_material(material_id);
}

Color FragShader::run(int frag_index )
{
    auto frag = &(*this->fragment_map)[frag_index];
    auto& alpha = frag->alpha;
    // auto& v1 = frag->triangle->vert[0];
    // auto& v2 = frag->triangle->vert[1];
    // auto& v3 = frag->triangle->vert[2];
    // Vec3 c1, c2, c3;
    // v1.get_attribute_value(1, c1);
    // v2.get_attribute_value(1, c2);
    // v3.get_attribute_value(1, c3);
    // Vec3 frag_color = c1 * alpha[0] + c2 * alpha[1] + c3 * alpha[2];
    return GREEN;
}

Color TextureFragShader::run(int frag_index)
{
    auto frag = &(*this->fragment_map)[frag_index];
    auto& v1 = frag->triangle->vert[0];
    auto texture = v1.attributes->textures[0];
    Vec4 frag_color;
    this->sample_texture(frag_index, texture, this->text_attribute_index, frag_color);
    return color(frag_color);
}




