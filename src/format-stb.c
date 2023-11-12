/*
MIT License

Copyright (c) 2019-2021 Andre Seidelt <superilu@yahoo.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#include "main.h"
#include "format-stb.h"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_SIMD
#define STBI_NO_JPEG
#define STBI_NO_PNG
#define STBI_NO_BMP
#define STBI_NO_TGA
#define STBI_NO_GIF
#define STBI_NO_PNM
#define STBI_NO_THREAD_LOCALS
#include "stb_image.h"

#define NUM_CHANNELS 4  //!< always use RGBA

/**
 * @brief load from file system
 *
 * @param filename the name of the file
 * @param pal pallette (is ignored)
 * @return BITMAP* or NULL if loading fails
 */
BITMAP *load_stb(AL_CONST char *filename, RGB *pal) {
    int width;
    int height;
    int channels_in_file;
    uint8_t *rgba = stbi_load(filename, &width, &height, &channels_in_file, NUM_CHANNELS);

    DEBUGF("image is %dx%dx%d\n", width, height, channels_in_file);

    // create bitmap
    BITMAP *bm = create_bitmap_ex(32, width, height);
    if (!bm) {
        stbi_image_free(rgba);
        return NULL;
    }

    // copy RGBA data in BITMAP
    for (int y = 0; y < height; y++) {
        unsigned int yIdx = y * NUM_CHANNELS * width;
        for (int x = 0; x < width; x++) {
            unsigned int xIdx = x * NUM_CHANNELS;
            bm->line[y][x * 4 + _rgb_r_shift_32 / 8] = rgba[0 + yIdx + xIdx];
            bm->line[y][x * 4 + _rgb_g_shift_32 / 8] = rgba[1 + yIdx + xIdx];
            bm->line[y][x * 4 + _rgb_b_shift_32 / 8] = rgba[2 + yIdx + xIdx];
            bm->line[y][x * 4 + _rgb_a_shift_32 / 8] = rgba[3 + yIdx + xIdx];
        }
    }

    stbi_image_free(rgba);

    return bm;
}
