#ifndef SRC_ENGINE_TEXTUREPNG_H_
#define SRC_ENGINE_TEXTUREPNG_H_

#include <string>
#include <cstdio>
#include <GL/glew.h>
#include <png.h>


inline GLuint loadPNG (const std::string &filename) {

    FILE *png_file = fopen(filename.c_str(), "rb");
    int row_bytes, bit_depth, color_type;
    unsigned width, height;
    png_byte *gl_image;
    png_bytep *png_image;
    GLuint texture;
    GLint format;
    png_uint_32 png_width, png_height;
    png_byte header[8];

    fread(header, 1, 8, png_file);

    if (png_sig_cmp(header, 0, 8)) {
       fclose(png_file);
       throw std::string("Image format not supported");
    }

    png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    png_infop
        info = png_create_info_struct(png),
        _tmp = png_create_info_struct(png);
    png_init_io(png, png_file);

    png_set_sig_bytes(png, 8);

    png_read_info(png, info);

    png_get_IHDR(png, info, &png_width, &png_height, &bit_depth, &color_type, nullptr, nullptr, nullptr);

    width = png_width;
    height = png_height;

    png_read_update_info(png, info);

    row_bytes = png_get_rowbytes(png, info);

    gl_image = new png_byte[row_bytes * height];
    png_image = new png_bytep[height];

    for (unsigned i = 0; i < height; i++) {
        png_image[height - 1 - i] = gl_image + (i * row_bytes);
    }

    png_read_image(png, png_image);

    format = color_type == PNG_COLOR_TYPE_RGB_ALPHA ? GL_RGBA : GL_RGB;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, gl_image);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    png_destroy_read_struct(&png, &info, &_tmp);

    delete[] png_image;
    delete[] gl_image;

    fclose(png_file);

    return texture;
}

#endif
