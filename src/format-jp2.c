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

    DEBUGF("LOAD: jasper initialized\n");

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
        DEBUGF("Can't change colorspace 1\n");
        jas_image_destroy(image);
        jas_cleanup_thread();
        jas_cleanup_library();
        return NULL;
    };
    if (!(altimage = jas_image_chclrspc(image, outprof, JAS_CMXFORM_INTENT_PER))) {
        DEBUGF("Can't change colorspace 2\n");
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
        DEBUGF("Can't create bitmap\n");
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
        DEBUGF("Can't create components\n");
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

int save_jp2(AL_CONST char *filename, BITMAP *bm, AL_CONST RGB *pal) {
    char outopts[100];
    snprintf(outopts, sizeof(outopts), "rate=%f", (float)output_quality / 100.0f);

    DEBUGF("Encoder options: %s\n", outopts);

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
        return -1;
    }
    if (jas_init_thread()) {
        DEBUGF("cannot initialize thread\n");
        jas_cleanup_library();
        return -1;
    }

    DEBUGF("WRITE: jasper initialized\n");

    int outfmt;
    if ((outfmt = jas_image_fmtfromname(filename)) < 0) {
        DEBUGF("Unknown format for %s\n", filename);
        jas_cleanup_thread();
        jas_cleanup_library();
        return -1;
    }

    jas_stream_t *out;
    if (!(out = jas_stream_fopen(filename, "w+b"))) {
        DEBUGF("error: cannot open output image file %s\n", filename);
        jas_cleanup_thread();
        jas_cleanup_library();
        return -1;
    }

    /* Create an image of the correct size. */
    const int NUM_COMPONENTS = 3;
    jas_image_t *image;
    jas_image_cmptparm_t cmptparms[3];
    for (int i = 0; i < NUM_COMPONENTS; ++i) {
        cmptparms[i].tlx = 0;
        cmptparms[i].tly = 0;
        cmptparms[i].hstep = 1;
        cmptparms[i].vstep = 1;
        cmptparms[i].width = bm->w;
        cmptparms[i].height = bm->h;
        cmptparms[i].prec = 8;
        cmptparms[i].sgnd = false;
    }
    if (!(image = jas_image_create(NUM_COMPONENTS, cmptparms, JAS_CLRSPC_UNKNOWN))) {
        DEBUGF("error: cannot create image\n");
        jas_cleanup_thread();
        jas_cleanup_library();
        return -1;
    }

    jas_image_setclrspc(image, JAS_CLRSPC_SRGB);
    jas_image_setcmpttype(image, 0, JAS_IMAGE_CT_COLOR(JAS_CLRSPC_CHANIND_RGB_R));
    jas_image_setcmpttype(image, 1, JAS_IMAGE_CT_COLOR(JAS_CLRSPC_CHANIND_RGB_G));
    jas_image_setcmpttype(image, 2, JAS_IMAGE_CT_COLOR(JAS_CLRSPC_CHANIND_RGB_B));

    jas_matrix_t *data[3];
    for (int cmptno = 0; cmptno < NUM_COMPONENTS; ++cmptno) {
        if (!(data[cmptno] = jas_matrix_create(1, bm->w))) {
            DEBUGF("error: cannot create matrix\n");
            jas_image_destroy(image);
            jas_cleanup_thread();
            jas_cleanup_library();
            return -1;
        }
    }

    for (int y = 0; y < bm->h; ++y) {
        for (int x = 0; x < bm->w; ++x) {
            uint32_t argb = getpixel(bm, x, y);
            int_fast64_t r = (argb >> 16) & 0xFF;
            int_fast64_t g = (argb >> 8) & 0xFF;
            int_fast64_t b = argb & 0xFF;

            jas_matrix_set(data[0], 0, x, r);
            jas_matrix_set(data[1], 0, x, g);
            jas_matrix_set(data[2], 0, x, b);
        }

        for (int cmptno = 0; cmptno < NUM_COMPONENTS; ++cmptno) {
            if (jas_image_writecmpt(image, cmptno, 0, y, bm->w, 1, data[cmptno])) {
                DEBUGF("error: cannot write component\n");
                for (int cmptno = 0; cmptno < NUM_COMPONENTS; ++cmptno) {
                    jas_matrix_destroy(data[cmptno]);
                }
                jas_image_destroy(image);
                jas_stream_close(out);
                jas_cleanup_thread();
                jas_cleanup_library();
                return -1;
            }
        }
    }

    if (jas_image_encode(image, out, outfmt, outopts)) {
        DEBUGF("error: cannot encode image\n");
        for (int cmptno = 0; cmptno < NUM_COMPONENTS; ++cmptno) {
            jas_matrix_destroy(data[cmptno]);
        }

        jas_image_destroy(image);
        jas_stream_close(out);
        jas_cleanup_thread();
        jas_cleanup_library();
        return -1;
    }
    jas_stream_flush(out);
    jas_image_destroy(image);

    for (int cmptno = 0; cmptno < NUM_COMPONENTS; ++cmptno) {
        jas_matrix_destroy(data[cmptno]);
    }

    /* Close the output image stream. */
    if (jas_stream_close(out)) {
        DEBUGF("error: cannot close output image file\n");
        jas_cleanup_thread();
        jas_cleanup_library();
        return -1;
    }

    jas_cleanup_thread();
    jas_cleanup_library();

    return 0;
}
