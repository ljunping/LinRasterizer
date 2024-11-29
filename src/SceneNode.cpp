//
// Created by Lin on 2024/11/16.
//

#include "SceneNode.h"

SceneNode::SceneNode():local_pos(Vec4::ZERO),global_pos(Vec4::ZERO),local_to_global_mat(Mat44::IDENTITY){};

SceneNode::~SceneNode()
{
}

void SceneNode::update()
{
}

void SceneNode::add_child(SceneNode* node)
{
}

void SceneNode::add_component(int component_type)
{
}

void SceneNode::remove_component(Component* c)
{
}

Component* SceneNode::get_component(int component_type)
{
}
