
#include "main.h"
#include "format-qoi.h"

#define QOI_IMPLEMENTATION
#include "qoi.h"

#define NUM_CHANNELS 4  //!< always use RGBA

/*

32bit
[DEBUG] QOI is 640x480
[DEBUG] _rgb_r_shift_32 is 16
[DEBUG] _rgb_g_shift_32 is 8
[DEBUG] _rgb_b_shift_32 is 0
[DEBUG] _rgb_a_shift_32 is 24
OK

24bit
[DEBUG] QOI is 640x480
[DEBUG] _rgb_r_shift_32 is 0
[DEBUG] _rgb_g_shift_32 is 8
[DEBUG] _rgb_b_shift_32 is 16
[DEBUG] _rgb_a_shift_32 is 24
OK

*/

/**
 * @brief load from file system
 *
 * @param filename the name of the file
 * @param pal pallette (is ignored)
 * @return BITMAP* or NULL if loading fails
 */
BITMAP *load_qoi(AL_CONST char *filename, RGB *pal) {
    qoi_desc desc;
    uint8_t *rgba = qoi_read(filename, &desc, NUM_CHANNELS);
    if (rgba) {
        DEBUGF("QOI is %dx%d\n", desc.width, desc.height);

        DEBUGF("_rgb_r_shift_32 is %d\n", _rgb_r_shift_32);
        DEBUGF("_rgb_g_shift_32 is %d\n", _rgb_g_shift_32);
        DEBUGF("_rgb_b_shift_32 is %d\n", _rgb_b_shift_32);
        DEBUGF("_rgb_a_shift_32 is %d\n", _rgb_a_shift_32);

        // create bitmap
        BITMAP *bm = create_bitmap_ex(32, desc.width, desc.height);
        if (!bm) {
            QOI_FREE(rgba);
            return NULL;
        }

        // copy RGBA data in BITMAP
        for (int y = 0; y < desc.height; y++) {
            unsigned int yIdx = y * NUM_CHANNELS * desc.width;
            for (int x = 0; x < desc.width; x++) {
                unsigned int xIdx = x * NUM_CHANNELS;
                bm->line[y][x * 4 + _rgb_r_shift_32 / 8] = rgba[0 + yIdx + xIdx];
                bm->line[y][x * 4 + _rgb_g_shift_32 / 8] = rgba[1 + yIdx + xIdx];
                bm->line[y][x * 4 + _rgb_b_shift_32 / 8] = rgba[2 + yIdx + xIdx];
                bm->line[y][x * 4 + _rgb_a_shift_32 / 8] = rgba[3 + yIdx + xIdx];
            }
        }

        QOI_FREE(rgba);

        return bm;
    } else {
        return NULL;
    }
}

/**
 * @brief convert BITMAP to rgba buffer and save as QOI
 *
 * @param bm BITMAP
 * @param fname file name
 * @return true for success, else false
 */
int save_qoi(AL_CONST char *fname, BITMAP *bm, AL_CONST RGB *pal) {
    int ret = -1;

    DEBUGF("save_qoi %s %dx%d\n", fname, bm->w, bm->h);

    uint8_t *rgba = malloc(bm->w * bm->h * NUM_CHANNELS);
    if (!rgba) {
        return ret;
    }

    DEBUGF("RGBA OK\n");

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

    DEBUGF("RGBA converted\n");

    ret = qoi_write(fname, rgba, &(qoi_desc){.width = bm->w, .height = bm->h, .channels = NUM_CHANNELS, .colorspace = QOI_SRGB}) != 0;

    DEBUGF("qoi_write = %d\n", ret);

    free(rgba);
    return !ret;
}
