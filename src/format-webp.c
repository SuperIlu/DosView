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
#include "util.h"
#include "format-webp.h"

#include "webp/decode.h"
#include "webp/encode.h"

#define NUM_CHANNELS 4  //!< always use RGBA

/**
 * @brief load from file system
 *
 * @param filename the name of the file
 * @param pal pallette (is ignored)
 *
 * @return BITMAP* or NULL if loading fails
 */
BITMAP *load_webp(AL_CONST char *filename, RGB *pal) {
    uint8_t *buffer;
    size_t size;

    DEBUGF("trying %s\n", filename);

    if (!ut_read_file(filename, (void **)&buffer, &size)) {
        return NULL;
    }

    DEBUGF("loaded %p, size %d\n", buffer, size);

    int width, height;
    uint8_t *rgba = WebPDecodeRGBA(buffer, size, &width, &height);
    if (rgba) {
        DEBUGF("WEBP is %dx%d\n", width, height);
        // create bitmap
        BITMAP *bm = create_bitmap_ex(32, width, height);
        if (!bm) {
            WebPFree(rgba);
            free(buffer);
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

        WebPFree(rgba);
        free(buffer);

        return bm;
    } else {
        free(buffer);
        return NULL;
    }
}

/**
 * @brief convert BITMAP to rgba buffer and save as losless webp
 *
 * @param bm BITMAP
 * @param fname file name
 *
 * @return true for success, else false
 */
int save_webp(AL_CONST char *fname, BITMAP *bm, AL_CONST RGB *pal) {
    int ret = -1;

    uint8_t *rgba = malloc(bm->w * bm->h * NUM_CHANNELS);
    if (!rgba) {
        return ret;
    }

    uint8_t *ptr = rgba;
    for (int y = 0; y < bm->h; y++) {
        for (int x = 0; x < bm->w; x++) {
            uint32_t argb = getpixel(bm, x, y);
            uint8_t a = 0xFF;
            uint8_t r = (argb >> 16) & 0xFF;
            uint8_t g = (argb >> 8) & 0xFF;
            uint8_t b = argb & 0xFF;

            *ptr++ = r;
            *ptr++ = g;
            *ptr++ = b;
            *ptr++ = a;
        }
    }

    uint8_t *output;
    // size_t size = WebPEncodeLosslessRGBA(rgba, bm->w, bm->h, bm->w * NUM_CHANNELS, &output); // this uses to much memory!
    size_t size = WebPEncodeRGBA(rgba, bm->w, bm->h, bm->w * NUM_CHANNELS, output_quality, &output);
    if (size > 0) {
        FILE *out = fopen(fname, "wb");
        if (out) {
            size_t num_written = fwrite(output, 1, size, out);
            ret = (num_written != size) ? -1 : 0;
            fclose(out);
        } else {
            DEBUGF("Could not create %s\n", fname);
            ret = false;
        }
        WebPFree(output);
    } else {
        ret = -1;
    }

    free(rgba);
    return ret;
}
