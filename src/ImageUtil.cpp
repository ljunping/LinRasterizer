//
// Created by Lin on 2025/1/1.
//

#include "ImageUtil.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#undef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#undef STB_IMAGE_WRITE_IMPLEMENTATION

// 假设 data 是你的图像数据，width 和 height 是图像的宽度和高度
// channels 是通道数（例如，3 表示 RGB，4 表示 RGBA）


Image ImageUtil::load_image(const char* path)
{
    Image image{};
    image.data = stbi_load(path, &image.width, &image.height, &image.channels, 0);
    if (!image.data)
    {
        printf("Failed to load texture %s\n", path);
    }
    return image;
}

void ImageUtil::destroy_image(unsigned char* image)
{
    stbi_image_free(image);
}


void ImageUtil::save_image(const char* path, Image& image)
{
    stbi_write_png(path, image.width, image.height, image.channels, image.data, image.width * image.channels);
}
