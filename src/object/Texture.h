//
// Created by Lin on 2024/11/27.
//

#ifndef TEXTURE_H
#define TEXTURE_H
#include "L_math.h"
#include "Resource.h"
#include "TrianglePrimitive.h"
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
    static void execute_init_lay0_mipmap(size_t begin, size_t end, void* global);
    static void execute_init_lay_mipmap(size_t data_begin, size_t data_end, void* global_data);
protected:
    Texture(const char* fileName, bool generate_mipmap);
    Texture(std::shared_ptr<unsigned char[]>& data,int w,int h,int channel, bool generate_mipmap);
    void texture_raw(int x, int y, const unsigned char*& result);
    void texture_raw(const Vec2& uv, const unsigned char*& result);
    void texture_mipmap_minify(int x, int y, int level, const unsigned char*& result);
    void texture_mipmap_magnify(int x, int y, int level, L_MATH::Vec<float, 4>& result);
    void texture_mipmap_minify(const L_MATH::Vec<float, 2>& uv, int level, L_MATH::Vec<float, 4>& result);
    void texture_mipmap_minify_linear(const L_MATH::Vec<float, 2>& uv, int level, L_MATH::Vec<float, 4>& result);
    void texture_mipmap_magnify(const L_MATH::Vec<float, 2>& uv, int level, L_MATH::Vec<float, 4>& result);
    void generate_mipmaps();
    void texture_mipmap_magnify_linear(const L_MATH::Vec<float, 2>& uv, int level, Vec4& result);
    void sample_linear(const L_MATH::Vec<float, 2>& uv, float lod, Vec4& result);
    bool mipmap = false;
    std::vector<TextureLayer> mip_maps;
public:
    ~Texture() override;
    int width = 0;
    int height = 0;
    int channels = 0;
    bool linear = false;
    virtual void sample(const L_MATH::Vec<float, 2>& uv, const L_MATH::Vec<float, 2>& lodv, L_MATH::Vec<float, 4>& result);
    virtual void sample(const L_MATH::Vec<float, 2>& uv, float lod, L_MATH::Vec<float, 4>& result);
};

class PureColorTexture : public Texture
{
    INIT_TYPE(PureColorTexture, Texture)
    Vec4 color;

protected:
    explicit PureColorTexture(const Vec4& color);

public:
    void sample(const L_MATH::Vec<float, 2>& uv, float lod, L_MATH::Vec<float, 4>& result) override;
    void sample(const L_MATH::Vec<float, 2>& uv, const L_MATH::Vec<float, 2>& lodv,
                L_MATH::Vec<float, 4>& result) override;
};




#endif //TEXTURE_H
