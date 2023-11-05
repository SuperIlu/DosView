#include "main.h"
#include "jasper/jasper.h"
#include "format-jp2.h"

#ifdef DEBUG_ENABLED
static int jp2_vlogmsgf_stdout(jas_logtype_t type, const char *fmt, va_list ap) {
    JAS_UNUSED(type);
    int result = vfprintf(stdout, fmt, ap);
    return result;
}
#endif

BITMAP *load_jp2(AL_CONST char *filename, RGB *pal) {
    // init jasper
    jas_conf_clear();
    static jas_std_allocator_t allocator;
    jas_std_allocator_init(&allocator);
    jas_conf_set_allocator(&allocator.base);
    jas_conf_set_debug_level(0);
    jas_conf_set_max_mem_usage(JAS_DEFAULT_MAX_MEM_USAGE);

#ifdef DEBUG_ENABLED
    jas_conf_set_debug_level(99);
    jas_conf_set_vlogmsgf(jp2_vlogmsgf_stdout);
#endif

    if (jas_init_library()) {
        DEBUGF("cannot initialize JasPer library\n");
        return NULL;
    }
    if (jas_init_thread()) {
        DEBUGF("cannot initialize thread\n");
        jas_cleanup_library();
        return NULL;
    }

    DEBUGF("jasper initialized\n");

    // open image stream
    jas_stream_t *in;
    if (!(in = jas_stream_fopen(filename, "rb"))) {
        DEBUGF("error: cannot open input image file %s\n", filename);
        jas_cleanup_thread();
        jas_cleanup_library();
        return NULL;
    }

    // load image data
    jas_image_t *image;
    if (!(image = jas_image_decode(in, -1, ""))) {
        DEBUGF("error: cannot load image data\n");
        jas_cleanup_thread();
        jas_cleanup_library();
        return NULL;
    }
    jas_stream_close(in);

    int components = jas_image_numcmpts(image);
    DEBUGF("num components = %d\n", components);

    if (components != 3) {
        DEBUGF("error: wrong number of components: %d\n", components);
        jas_cleanup_thread();
        jas_cleanup_library();
        return NULL;
    }

    int width = jas_image_width(image);
    int height = jas_image_height(image);

    DEBUGF("width  = %d\n", width);
    DEBUGF("height = %d\n", height);

    // convert colors
    jas_cmprof_t *outprof = NULL;
    jas_image_t *altimage;
    if (!(outprof = jas_cmprof_createfromclrspc(JAS_CLRSPC_SRGB))) {
        jas_image_destroy(image);
        jas_cleanup_thread();
        jas_cleanup_library();
        return NULL;
    };
    if (!(altimage = jas_image_chclrspc(image, outprof, JAS_CMXFORM_INTENT_PER))) {
        jas_cmprof_destroy(outprof);
        jas_image_destroy(image);
        jas_cleanup_thread();
        jas_cleanup_library();
        return NULL;
    };
    jas_cmprof_destroy(outprof);

    // create bitmap
    BITMAP *bm = create_bitmap_ex(32, width, height);
    if (!bm) {
        jas_image_destroy(image);
        jas_image_destroy(altimage);
        jas_cleanup_thread();
        jas_cleanup_library();
        return NULL;
    }
    DEBUGF("bm = %p\n", bm);

    int comp_r, comp_g, comp_b;
    if ((comp_r = jas_image_getcmptbytype(altimage, JAS_IMAGE_CT_COLOR(JAS_CLRSPC_CHANIND_RGB_R))) < 0 ||
        (comp_g = jas_image_getcmptbytype(altimage, JAS_IMAGE_CT_COLOR(JAS_CLRSPC_CHANIND_RGB_G))) < 0 ||
        (comp_b = jas_image_getcmptbytype(altimage, JAS_IMAGE_CT_COLOR(JAS_CLRSPC_CHANIND_RGB_B))) < 0) {
        jas_image_destroy(image);
        jas_image_destroy(altimage);
        jas_cleanup_thread();
        jas_cleanup_library();
        return NULL;
    }

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            bm->line[y][x * 4 + _rgb_r_shift_32 / 8] = jas_image_readcmptsample(image, comp_r, x, y);
            bm->line[y][x * 4 + _rgb_g_shift_32 / 8] = jas_image_readcmptsample(image, comp_g, x, y);
            bm->line[y][x * 4 + _rgb_b_shift_32 / 8] = jas_image_readcmptsample(image, comp_b, x, y);
            bm->line[y][x * 4 + _rgb_a_shift_32 / 8] = 0xFF;
        }
    }

    jas_image_destroy(image);
    jas_image_destroy(altimage);
    jas_cleanup_thread();
    jas_cleanup_library();

    return bm;
}
