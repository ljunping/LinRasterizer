//
// Created by Lin on 2024/11/16.
//

#ifndef SCENENODE_H
#define SCENENODE_H
#include "L_math.h"


class Component;

class SceneNode
{
public:
    Vec4 local_pos;
    Vec4 global_pos;
    Mat44 local_to_global_mat;
    SceneNode* parent;
    SceneNode* child;
    int child_count;
    SceneNode();
    virtual ~SceneNode();
    void update();
    void add_child(SceneNode* node);
    void add_component(int component_type);
    void remove_component(Component* c);
    Component* get_component(int component_type);
};



#endif //SCENENODE_H
