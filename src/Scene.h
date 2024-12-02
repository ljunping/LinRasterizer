//
// Created by Lin on 2024/11/15.
//

#ifndef SCENE_H
#define SCENE_H
#include <unordered_map>
#include <vector>

#include "L_math.h"
#include "TrianglePrimitive.h"
#include "VectorRemoveEasy.h"


class Scene {

private:
    VectorRemoveEasy<TrianglePrimitive*> model_triangles;
    std::unordered_map<int, TrianglePrimitive*> scene_triangle_map;
    int next_triangle_id();
    int current_triangle_id = 0;
public:
    Mat44 model_matrix;
    Scene();
    ~Scene();
    void update_model_matrix(const Mat44& mat);
    int add_origin_triangle(TrianglePrimitive& tri);
    void add_origin_triangles(std::vector<TrianglePrimitive>& tris);
    int add_origin_triangles(std::vector<TrianglePrimitive*>& tris);
    int add_attributes(Attributes* attrs);

    void remove_origin_triangle(int id);
    void clear();
    void update_origin_triangle(int id, TrianglePrimitive* tri);
    VectorRemoveEasy<TrianglePrimitive*>& get_model_triangle_list();
    int get_triangle_count() const;
};


#endif //SCENE_H
