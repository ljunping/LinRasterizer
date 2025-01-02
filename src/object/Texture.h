//
// Created by Lin on 2024/11/27.
//

#ifndef TEXTURE_H
#define TEXTURE_H
#include "L_math.h"
#include "Resource.h"
#include "TrianglePrimitive.h"
#define MAX_TEXTURES 8
struct TextureLayer
{
    int width;
    int height;
    unsigned char *data;
};

class Texture : public Resource
{
    INIT_TYPE(Texture, Resource)
    SHARE_PTR<unsigned char[]> data;
public:
    ~Texture() override;
    int width = 0;
    int height = 0;
    int channels = 0;
    bool mipmap = false;
    bool linear = false;
    std::vector<TextureLayer> mip_maps;
    void texture_raw(int x, int y, const unsigned char*& result);
    void texture_raw(const Vec2& uv, const unsigned char*& result);

    void texture_mipmap_minify(int x, int y, int level, const unsigned char*& result);
    void texture_mipmap_magnify(int x, int y, int level, unsigned char*& result);
    void texture_mipmap_minify(const L_MATH::Vec<float, 2>& uv, int level, unsigned char*& result);
    void texture_mipmap_minify_linear(const L_MATH::Vec<float, 2>& uv, int level, unsigned char*& result);
    void texture_mipmap_magnify(const Vec2& uv, int level, unsigned char*& result);
    void texture_mipmap_magnify_linear(const Vec2& uv, int level, unsigned char*& result);
    void generate_mipmaps();
    void sample(const L_MATH::Vec<float, 2>& uv, const Vec2& lod, unsigned char* result);
    Texture(const char* fileName, bool generate_mipmap);
    Texture(std::shared_ptr<unsigned char[]>& data,int w,int h,int channel, bool generate_mipmap);
    void sample_linear(const L_MATH::Vec<float, 2>& uv, float lod, unsigned char* result);
    void sample(const L_MATH::Vec<float, 2>& uv, float lod, unsigned char* result);
}
;



#endif //TEXTURE_H
