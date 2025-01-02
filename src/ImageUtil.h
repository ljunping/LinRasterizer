//
// Created by Lin on 2025/1/1.
//

#ifndef IMAGEUTIL_H
#define IMAGEUTIL_H

struct Image
{
    unsigned char* data;
    int width;
    int height;
    int channels;
};

struct  ImageUtil {

    static Image load_image(const char* path);
    static void destroy_image(unsigned char* image);
    static void save_image(const char* path, Image& image);
};



#endif //IMAGEUTIL_H
