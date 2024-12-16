//
// Created by Lin on 2024/11/16.
//

#ifndef RENDERNODE_H
#define RENDERNODE_H
#include "L_math.h"
#include "Attribute.h"
#include "Component.h"
#include "FragShader.h"


class RenderNode {
    public:
    RenderNode();
    RenderNode(MeshRender* mesh_render);
    void render();
};



#endif //RENDERNODE_H
