#include "MeshGenerator.h"

#include "BresenhamLine.h"
#include "Context.h"
#include "Mesh.h"
#include "Object.h"

void draw_line(Context* ctx, Color* buff, void* data)
{
    int w, h;
    ctx->get_screen_size(w, h);
    auto line = (DrawLineInfo*)data;
    auto bresenham_line = BresenhamLine(line->x0, line->y0, line->x1, line->y1);
    int x, y, mx, my;
    x = line->x0;
    y = line->y0;
    buff[y * w + x] = line->color;
    while (bresenham_line.has_next())
    {
        bresenham_line.next_point(x, y, mx, my);
        buff[y * w + x] = line->color;
    }
}

Mesh* generate_sphere(float radius, int stacks, int slices)
{
    float* vert_buff = new float[8 * (stacks + 1) * (slices + 1)];

    // Generate vertices
    for (int stack = 0; stack < stacks; ++stack)
    {
        float phi = M_PI * stack / stacks; // Latitude angle (0 to π)
        for (int slice = 0; slice <= slices; ++slice)
        {
            float theta = 2 * M_PI * slice / slices; // Longitude angle (0 to 2π)
            float x = radius * sin(phi) * cos(theta);
            float y = radius * sin(phi) * sin(theta);
            float z = radius * cos(phi);
            vert_buff[8 * (stack * (slices + 1) + slice)] = x;
            vert_buff[8 * (stack * (slices + 1) + slice) + 1] = y;
            vert_buff[8 * (stack * (slices + 1) + slice) + 2] = z;

            vert_buff[8 * (stack * (slices + 1) + slice) + 3] = x;
            vert_buff[8 * (stack * (slices + 1) + slice) + 4] = y;
            vert_buff[8 * (stack * (slices + 1) + slice) + 5] = z;

            vert_buff[8 * (stack * (slices + 1) + slice) + 6] = stack*1.0f / stacks;
            vert_buff[8 * (stack * (slices + 1) + slice) + 7] = slice * 1.0f / slices;

        }
    }
    SHARE_PTR<float[]> share_vert_buff(vert_buff);
    auto _Attributes = CREATE_OBJECT_BY_TYPE(Mesh, share_vert_buff, 8 * (stacks + 1) * (slices + 1));
    _Attributes->bind_attribute(POS,3, 0, 8);
    _Attributes->bind_attribute(NORMAL,3, 3, 8);
    _Attributes->bind_attribute(UV, 2, 6, 8);
    std::vector<int> eb0;
    // Generate triangles
    for (int stack = 0; stack < stacks; ++stack)
    {
        for (int slice = 0; slice < slices; ++slice)
        {
            // Compute indices for the four vertices of the current quad
            int i1 = stack * (slices + 1) + slice; // Top-left
            int i2 = i1 + 1; // Top-right
            int i3 = i1 + (slices + 1); // Bottom-left
            int i4 = i3 + 1; // Bottom-right
            // Two triangles per quad
            if (stack != 0)
            {
                eb0.emplace_back(i1);
                eb0.emplace_back(i3);
                eb0.emplace_back(i2);
            }
            if (stack != stacks- 1)
            {
                eb0.emplace_back(i3);
                eb0.emplace_back(i4);
                eb0.emplace_back(i2);
            }
        }
    }
    _Attributes->ebo = std::move(eb0);
    _Attributes->calculate_tangents();
    return _Attributes;
}

Mesh* generate_quad()
{
    float* vert_buff = new float[4 * 8]
    {
        -0.5, -0.5, 0, 0, 0, 1, 0, 0,
        0.5, -0.5, 0, 0, 0, 1, 1, 0,
        -0.5, 0.5, 0, 0, 0, 1, 0, 1,
        0.5, 0.5, 0, 0, 0, 1, 1, 1,
    };

    SHARE_PTR<float[]> share_vert_buff(vert_buff);
    auto _Attributes = CREATE_OBJECT_BY_TYPE(Mesh, share_vert_buff, 4*8);
    _Attributes->bind_attribute(POS,3, 0, 8);
    _Attributes->bind_attribute(NORMAL,3, 3, 8);
    _Attributes->bind_attribute(UV, 2, 6, 8);
    _Attributes->ebo = {0, 1, 2, 1, 3, 2};
    _Attributes->calculate_tangents();
    return _Attributes;
}

Mesh* generate_tri()
{
    float* vert_buff = new float[4 * 8]
    {
        -0.5, -0.5, 0, 0, 0, 1, 0, 0,
        0.5, -0.5, 0, 0, 0, 1, 1, 0,
        -0.5, 0.5, 0, 0, 0, 1, 0, 1,
        0.5, 0.5, 0, 0, 0, 1, 1, 1,
    };

    SHARE_PTR<float[]> share_vert_buff(vert_buff);
    auto _Attributes = CREATE_OBJECT_BY_TYPE(Mesh, share_vert_buff, 4*8);
    _Attributes->bind_attribute(POS,3, 0, 8);
    _Attributes->bind_attribute(NORMAL,3, 3, 8);
    _Attributes->bind_attribute(UV, 2, 6, 8);
    _Attributes->ebo = {0, 1, 2};
    _Attributes->calculate_tangents();
    return _Attributes;
}
