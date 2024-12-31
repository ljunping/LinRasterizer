//
// Created by Lin on 2024/11/27.
//

#include "Texture.h"

#include "JobSystem.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

static void execute_init_lay0_mipmap(size_t begin, size_t end, void *global)
{
    Texture *texture = (Texture *)global;
    auto& mip_map_layer = texture->mip_maps[0];
    int wpt2 = mip_map_layer.width;
    int hpt2 = mip_map_layer.height;
    auto channels = texture->channels;
    auto mip_map = mip_map_layer.data;
    const unsigned char *tx0, *tx1, *tx2, *tx3;
    Vec4 alpha;
    for (int index = begin; index < end; ++index)
    {
        int j = index / wpt2;
        int i = index % wpt2;
        auto x = i * 1.0 / wpt2 * texture->width;
        auto y = j * 1.0 / hpt2 * texture->height;
        int x0 = floor(x);
        int y0 = floor(y);
        float alpha_x = x - x0;
        float alpha_y = y - y0;
        L_MATH::alpha4(alpha_x, alpha_y, alpha);
        texture->texture_raw(x0, y0, tx0);
        texture->texture_raw(x0 + 1, y0, tx1);
        texture->texture_raw(x0, y0 + 1, tx2);
        texture->texture_raw(x0 + 1, y0 + 1, tx3);
        for (int k = 0; k < channels; ++k)
        {
            mip_map[channels * index + k] = L_MATH::linear4(tx0[k], tx1[k], tx2[k], tx3[k], alpha);
        }
    }
}

void execute_init_lay_mipmap(size_t data_begin, size_t data_end, void * global_data)
{
    auto *data = (std::tuple<Texture *, int> *)(global_data);
    Texture* texture = std::get<0>(*data);
    int level = std::get<1>(*data);
    auto &texture_layers = texture->mip_maps;
    auto &texture_layer = texture_layers[level];
    int w = (texture_layer.width);
    const unsigned char *tx0, *tx1, *tx2, *tx3;
    Vec4 alpha{0.25, 0.25, 0.25, 0.25};
    for (int index = data_begin; index < data_end; ++index)
    {
        int j = index / w;
        int i = index % w;
        int oj = 2 * j;
        int oi = 2 * i;
        texture->texture_mipmap_minify(oi, oj, level - 1, tx0);
        texture->texture_mipmap_minify(oi + 1, oj, level - 1, tx1);
        texture->texture_mipmap_minify(oi, oj + 1, level - 1, tx2);
        texture->texture_mipmap_minify(oi + 1, oj + 1, level - 1, tx3);
        for (int k = 0; k < texture->channels; ++k)
        {
            texture_layer.data[texture->channels * index + k] = L_MATH::linear4(tx0[k], tx1[k], tx2[k], tx3[k], alpha);
        }
    }

}


Texture::~Texture()
{
    for (auto mip_map : mip_maps)
    {
        free(mip_map.data);
    }
}


void Texture::texture_raw(int x, int y, const unsigned char *&result)
{
    int index = std::max(std::min((height - y) * width + x, width * height - 1), 0) * this->channels;
    result = (data.get() + index);
}

void Texture::texture_raw(const L_MATH::Vec<float, 2>& uv, const unsigned char*& result)
{
    auto x = floor(uv[0] * width);
    auto y = floor(uv[1] * height);
    texture_raw(x, y, result);
}

void Texture::texture_mipmap_minify(int x, int y, int level, const unsigned char *&result)
{
    auto &mip_map = mip_maps[level];
    int index = std::max(
        std::min((y * mip_map.width + x), mip_map.width * mip_map.height - 1), 0) * this->channels;
    result = mip_map.data + index;
}

void Texture::texture_mipmap_magnify(int x, int y, int level, unsigned char *&result)
{
    float delta = 1.0 / (1 << level);
    auto lay0_x = x >> level;
    auto lay0_y = y >> level;
    const unsigned char *tx0, *tx1, *tx2, *tx3;
    this->texture_mipmap_minify(lay0_x, lay0_y, 0, tx0);
    this->texture_mipmap_minify(lay0_x + 1, lay0_y, 0, tx1);
    this->texture_mipmap_minify(lay0_x, lay0_y + 1, 0, tx2);
    this->texture_mipmap_minify(lay0_x + 1, lay0_y + 1, 0, tx3);
    float alpha_x = delta * (x - (lay0_x << level));
    float alpha_y = delta * (y - (lay0_y << level));
    Vec4 alpha;
    L_MATH::alpha4(alpha_x, alpha_y, alpha);
    for (int i = 0; i < channels; ++i)
    {
        result[i] = L_MATH::linear4(tx0[i], tx1[i], tx2[i], tx3[i], alpha);
    }
}

void Texture::texture_mipmap_minify(const L_MATH::Vec<float, 2> &uv, int level,  unsigned char *&result)
{
    level = std::min(level, (int)(mip_maps.size() - 1));
    auto height = mip_maps[level].height;
    auto width = mip_maps[level].width;
    auto x = floor(uv[0] * width);
    auto y = floor(uv[1] * height);
    const unsigned char* res;
    texture_mipmap_minify(x, y, level, res);
    for (int i = 0; i < channels; ++i)
    {
        result[i] = res[i];
    }
}

void Texture::texture_mipmap_minify_linear(const L_MATH::Vec<float, 2>& uv, int level, unsigned char*& result)
{
    level = std::min(level, (int)(mip_maps.size() - 1));
    auto height = mip_maps[level].height;
    auto width = mip_maps[level].width;
    auto x = floor(uv[0] * width);
    auto y = floor(uv[1] * height);
    float alpha_x = uv[0] * width - x;
    float alpha_y = uv[1] * height - y;
    Vec4 alpha;
    L_MATH::alpha4(alpha_x, alpha_y, alpha);
    const unsigned char *tx0, *tx1, *tx2, *tx3;
    texture_mipmap_minify(x, y, level, tx0);
    texture_mipmap_minify(x + 1, y, level, tx1);
    texture_mipmap_minify(x, y + 1, level, tx2);
    texture_mipmap_minify(x + 1, y + 1, level, tx3);
    for (int i = 0; i < channels; ++i)
    {
        result[i] = L_MATH::linear4(tx0[i], tx1[i], tx2[i], tx3[i], alpha);
    }
}

void Texture::texture_mipmap_magnify(const L_MATH::Vec<float, 2> &uv, int level, unsigned char *&result)
{
    if(level==0)
    {
        texture_mipmap_minify(uv, level, result);
        return;
    }
    auto height = mip_maps[0].height << level;
    auto width = mip_maps[0].width << level;
    auto x = floor(uv[0] * width);
    auto y = floor(uv[1] * height);
    texture_mipmap_magnify(x, y, level, result);
}

void Texture::texture_mipmap_magnify_linear(const L_MATH::Vec<float, 2>& uv, int level, unsigned char*& result)
{
    if(level==0)
    {
        texture_mipmap_minify_linear(uv, level, result);
        return;
    }
    auto height = mip_maps[0].height << level;
    auto width = mip_maps[0].width << level;
    auto x = floor(uv[0] * width);
    auto y = floor(uv[1] * height);
    float alpha_x = uv[0] * width - x;
    float alpha_y = uv[1] * height - y;
    Vec4 alpha;
    L_MATH::alpha4(alpha_x, alpha_y, alpha);

    auto *tx0=new unsigned char[this->channels];
    auto *tx1=new unsigned char[this->channels];
    auto *tx2=new unsigned char[this->channels];
    auto *tx3=new unsigned char[this->channels];

    texture_mipmap_magnify(x, y, level, tx0);
    texture_mipmap_magnify(x + 1, y, level, tx1);
    texture_mipmap_magnify(x, y + 1, level, tx2);
    texture_mipmap_magnify(x + 1, y + 1, level, tx3);
    for (int i = 0; i < channels; ++i)
    {
        result[i] = L_MATH::linear4(tx0[i], tx1[i], tx2[i], tx3[i], alpha);
    }
    delete[] tx0;
    delete[] tx1;
    delete[] tx2;
    delete[] tx3;
}


void Texture::generate_mipmaps()
{
    if(data == nullptr)
    {
        return;
    }
    this->mipmap = true;
    mip_maps.clear();
    auto wpt2 = L_MATH::ceil_pot(this->width);
    auto hpt2 = L_MATH::ceil_pot(this->height);
    auto level = std::max(INT_HIGHEST_BIT_INDEX(wpt2),INT_HIGHEST_BIT_INDEX(hpt2));
    mip_maps.resize(level + 1);
    auto &text_layer = mip_maps[0];
    text_layer.width = wpt2;
    text_layer.height = hpt2;
    int lay0_fence = 0;
    auto* mip_map_0 = static_cast<unsigned char*>(malloc(wpt2 * hpt2 * this->channels));
    text_layer.data = mip_map_0;
    lay0_fence = JOB_SYSTEM.create_job_group(0);
    JOB_SYSTEM.alloc_jobs(lay0_fence, 0, 0, wpt2 * hpt2, this, execute_init_lay0_mipmap, default_complete);
    JOB_SYSTEM.submit_job_group(lay0_fence);
    int lay_pre_fence = lay0_fence;
    std::tuple<Texture *, int> *global_data = new std::tuple<Texture *, int>[level];
    for (int i = 0; i < level; ++i)
    {
        std::get<0>(global_data[i]) = this;
        std::get<1>(global_data[i]) = i + 1;
    }
    for (int i = 1; i <= level; ++i)
    {
        auto& pre_mip_map = mip_maps[i - 1];
        int w = (pre_mip_map.width >> 1), h = (pre_mip_map.height >> 1);
        w = w == 0 ? 1 : w;
        h = h == 0 ? 1 : h;
        auto mip_map_i = static_cast<unsigned char *>(malloc(w * h * this->channels));
        auto &text_layer_i = mip_maps[i];
        text_layer_i.width = w;
        text_layer_i.height = h;
        text_layer_i.data = mip_map_i;
        auto lay_fence = JOB_SYSTEM.create_job_group(lay_pre_fence);
        JOB_SYSTEM.alloc_jobs(lay_fence, 0, 0, w * h, global_data + i - 1, execute_init_lay_mipmap, default_complete);
        JOB_SYSTEM.submit_job_group(lay_fence);
        lay_pre_fence = lay_fence;
    }
    JOB_SYSTEM.wait_job_group_finish(lay_pre_fence);
}

void Texture::sample(const L_MATH::Vec<float, 2> &uv, const Vec2 &lodv, unsigned char *result)
{
    if (!mipmap)
    {
        int x = floor(uv[0] * width);
        int y = floor(uv[1] * height);
        const unsigned char *tx0;
        texture_raw(x, y, tx0);
        for (int i = 0; i < channels; ++i)
        {
            result[i] = tx0[i];
        }
        return;
    }
    auto &mip_map = this->mip_maps[0];

    float lod = 0;
    float w = mip_map.width, h = mip_map.height;
    if (!L_MATH::is_zero(lodv[0]) || !L_MATH::is_zero(lodv[1]))
    {
        lod = -0.5f * log(std::max(lodv[0] * w * w,
                                   lodv[1] * h * h));
    }
    if (linear)
    {
        sample_linear(uv, lod, result);
    }else
    {
        sample(uv, lod, result);
    }
}

Texture::Texture(const char* fileName,bool generate_mipmap)
{
    int width;
    int height;
    int channels;
    unsigned char* imageData = stbi_load(fileName, &width, &height, &channels, 0);
    if (!imageData)
    {
        printf("Failed to load texture %s\n", fileName);
    }
    SHARE_PTR<unsigned char[]> data(imageData, [](unsigned char *ptr)
    {
        stbi_image_free(ptr);
    });
    this->height = height;
    this->width = width;
    this->channels = channels;
    this->data = data;
    if(generate_mipmap)
    {
        this->generate_mipmaps();
    }
}

void Texture::sample_linear(const L_MATH::Vec<float, 2>& uv, float lod, unsigned char* result)
{
    if (lod <= 0)
    {
        lod = abs(lod);
        int l = floor(lod);
        float alpha = lod - l;
        auto tx1 = new unsigned char[this->channels];
        auto tx2 = new unsigned char[this->channels];
        texture_mipmap_minify(uv, l, tx1);
        texture_mipmap_minify(uv, l + 1, tx2);
        for (int i = 0; i < channels; ++i)
        {
            result[i] = tx1[i] * (1 - alpha) + tx2[i] * alpha;
        }
        delete[] tx1;
        delete[] tx2;
    }
    else
    {
        int l = floor(lod);
        float alpha = lod - l;
        auto tx1 = new unsigned char[this->channels];
        auto tx2 = new unsigned char[this->channels];
        texture_mipmap_magnify_linear(uv, l, tx1);
        texture_mipmap_magnify_linear(uv, l + 1, tx2);
        for (int i = 0; i < channels; ++i)
        {
            result[i] = tx1[i] * (1 - alpha) + tx2[i] * alpha;
        }
        delete[] tx1;
        delete[] tx2;
    }
}

void Texture::sample(const L_MATH::Vec<float, 2> &uv, float lod, unsigned char *result)
{
    if (lod <= 0)
    {
        lod = abs(lod);
        int l = floor(lod);
        texture_mipmap_minify(uv, l, result);
    }
    else
    {
        int l = floor(lod);
        texture_mipmap_magnify(uv, l, result);
    }
}









