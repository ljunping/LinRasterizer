//
// Created by Lin on 2024/11/27.
//

#ifndef TEXTURE_H
#define TEXTURE_H
#include "L_math.h"
#include "TrianglePrimitive.h"
#define MAX_TEXTURES 8
struct TextureLayer
{
    int width;
    int height;
    unsigned char *data;
};
class Texture
{
    friend class TextureManager;
    int id;
    unsigned char *data;
    Texture();
public:
    ~Texture();
    int width = 0;
    int height = 0;
    int channels = 0;
    bool mipmap=false;
    std::vector<TextureLayer> mip_maps;
    void texture_raw(int x, int y, const unsigned char*& result);
    void texture_mipmap_minify(int x, int y, int level, const unsigned char*& result);
    void texture_mipmap_magnify(int x, int y, int level, unsigned char*& result);
    void texture_mipmap_minify(const Vec2& uv, int level,  unsigned char*& result);
    void texture_mipmap_magnify(const Vec2& uv, int level, unsigned char*& result);
    void generate_mipmaps();
    void sample(const L_MATH::Vec<float, 2>& uv, const Vec2& lod, unsigned char* result);
    void sample(const L_MATH::Vec<float, 2>& uv, float lod, unsigned char* result);

};

class TextureManager
{
    Texture* textures[MAX_TEXTURES];
    int textureID;
public:
    Texture* get_texture(int id);
    TextureManager();
    int create_texture(const char* fileName,bool generate_mipmap);
    void destroy_texture(int id);
    void sample(int texture_id, const L_MATH::Vec<float, 2>& uv, const L_MATH::Vec<float, 2>& lod, unsigned char* result);
    void sample(int texture_id, const L_MATH::Vec<float, 2>& uv, float lod, unsigned char* result);
};




inline TextureManager TEXTURE_MANAGER;



#endif //TEXTURE_H
