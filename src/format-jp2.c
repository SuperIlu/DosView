#include "main.h"
#include "openjpeg.h"
#include "format-jp2.h"

#ifndef USE_JASPER

static void error_callback(const char *msg, void *client_data) {
#ifdef DEBUG_ENABLED
    (void)client_data;
    fprintf(stdout, "[ERROR] %s", msg);
#endif
}
static void warning_callback(const char *msg, void *client_data) {
#ifdef DEBUG_ENABLED
    (void)client_data;
    fprintf(stdout, "[WARNING] %s", msg);
#endif
}
static void info_callback(const char *msg, void *client_data) {
#ifdef DEBUG_ENABLED
    (void)client_data;
    fprintf(stdout, "[INFO] %s", msg);
#endif
}

static void sycc_to_rgb(int offset, int upb, int y, int cb, int cr, int *out_r, int *out_g, int *out_b) {
    int r, g, b;

    cb -= offset;
    cr -= offset;
    r = y + (int)(1.402 * (float)cr);
    if (r < 0) {
        r = 0;
    } else if (r > upb) {
        r = upb;
    }
    *out_r = r;

    g = y - (int)(0.344 * (float)cb + 0.714 * (float)cr);
    if (g < 0) {
        g = 0;
    } else if (g > upb) {
        g = upb;
    }
    *out_g = g;

    b = y + (int)(1.772 * (float)cb);
    if (b < 0) {
        b = 0;
    } else if (b > upb) {
        b = upb;
    }
    *out_b = b;
}

static void sycc444_to_rgb(opj_image_t *img) {
    int *d0, *d1, *d2, *r, *g, *b;
    const int *y, *cb, *cr;
    size_t maxw, maxh, max, i;
    int offset, upb;

    upb = (int)img->comps[0].prec;
    offset = 1 << (upb - 1);
    upb = (1 << upb) - 1;

    maxw = (size_t)img->comps[0].w;
    maxh = (size_t)img->comps[0].h;
    max = maxw * maxh;

    y = img->comps[0].data;
    cb = img->comps[1].data;
    cr = img->comps[2].data;

    d0 = r = (int *)opj_image_data_alloc(sizeof(int) * max);
    d1 = g = (int *)opj_image_data_alloc(sizeof(int) * max);
    d2 = b = (int *)opj_image_data_alloc(sizeof(int) * max);

    if (r == NULL || g == NULL || b == NULL) {
        goto fails;
    }

    for (i = 0U; i < max; ++i) {
        sycc_to_rgb(offset, upb, *y, *cb, *cr, r, g, b);
        ++y;
        ++cb;
        ++cr;
        ++r;
        ++g;
        ++b;
    }
    opj_image_data_free(img->comps[0].data);
    img->comps[0].data = d0;
    opj_image_data_free(img->comps[1].data);
    img->comps[1].data = d1;
    opj_image_data_free(img->comps[2].data);
    img->comps[2].data = d2;
    img->color_space = OPJ_CLRSPC_SRGB;
    return;

fails:
    opj_image_data_free(r);
    opj_image_data_free(g);
    opj_image_data_free(b);
} /* sycc444_to_rgb() */

static void sycc422_to_rgb(opj_image_t *img) {
    int *d0, *d1, *d2, *r, *g, *b;
    const int *y, *cb, *cr;
    size_t maxw, maxh, max, offx, loopmaxw;
    int offset, upb;
    size_t i;

    upb = (int)img->comps[0].prec;
    offset = 1 << (upb - 1);
    upb = (1 << upb) - 1;

    maxw = (size_t)img->comps[0].w;
    maxh = (size_t)img->comps[0].h;
    max = maxw * maxh;

    y = img->comps[0].data;
    cb = img->comps[1].data;
    cr = img->comps[2].data;

    d0 = r = (int *)opj_image_data_alloc(sizeof(int) * max);
    d1 = g = (int *)opj_image_data_alloc(sizeof(int) * max);
    d2 = b = (int *)opj_image_data_alloc(sizeof(int) * max);

    if (r == NULL || g == NULL || b == NULL) {
        goto fails;
    }

    /* if img->x0 is odd, then first column shall use Cb/Cr = 0 */
    offx = img->x0 & 1U;
    loopmaxw = maxw - offx;

    for (i = 0U; i < maxh; ++i) {
        size_t j;

        if (offx > 0U) {
            sycc_to_rgb(offset, upb, *y, 0, 0, r, g, b);
            ++y;
            ++r;
            ++g;
            ++b;
        }

        for (j = 0U; j < (loopmaxw & ~(size_t)1U); j += 2U) {
            sycc_to_rgb(offset, upb, *y, *cb, *cr, r, g, b);
            ++y;
            ++r;
            ++g;
            ++b;
            sycc_to_rgb(offset, upb, *y, *cb, *cr, r, g, b);
            ++y;
            ++r;
            ++g;
            ++b;
            ++cb;
            ++cr;
        }
        if (j < loopmaxw) {
            sycc_to_rgb(offset, upb, *y, *cb, *cr, r, g, b);
            ++y;
            ++r;
            ++g;
            ++b;
            ++cb;
            ++cr;
        }
    }

    opj_image_data_free(img->comps[0].data);
    img->comps[0].data = d0;
    opj_image_data_free(img->comps[1].data);
    img->comps[1].data = d1;
    opj_image_data_free(img->comps[2].data);
    img->comps[2].data = d2;

    img->comps[1].w = img->comps[2].w = img->comps[0].w;
    img->comps[1].h = img->comps[2].h = img->comps[0].h;
    img->comps[1].dx = img->comps[2].dx = img->comps[0].dx;
    img->comps[1].dy = img->comps[2].dy = img->comps[0].dy;
    img->color_space = OPJ_CLRSPC_SRGB;
    return;

fails:
    opj_image_data_free(r);
    opj_image_data_free(g);
    opj_image_data_free(b);
} /* sycc422_to_rgb() */

static void sycc420_to_rgb(opj_image_t *img) {
    int *d0, *d1, *d2, *r, *g, *b, *nr, *ng, *nb;
    const int *y, *cb, *cr, *ny;
    size_t maxw, maxh, max, offx, loopmaxw, offy, loopmaxh;
    int offset, upb;
    size_t i;

    upb = (int)img->comps[0].prec;
    offset = 1 << (upb - 1);
    upb = (1 << upb) - 1;

    maxw = (size_t)img->comps[0].w;
    maxh = (size_t)img->comps[0].h;
    max = maxw * maxh;

    y = img->comps[0].data;
    cb = img->comps[1].data;
    cr = img->comps[2].data;

    d0 = r = (int *)opj_image_data_alloc(sizeof(int) * max);
    d1 = g = (int *)opj_image_data_alloc(sizeof(int) * max);
    d2 = b = (int *)opj_image_data_alloc(sizeof(int) * max);

    if (r == NULL || g == NULL || b == NULL) {
        goto fails;
    }

    /* if img->x0 is odd, then first column shall use Cb/Cr = 0 */
    offx = img->x0 & 1U;
    loopmaxw = maxw - offx;
    /* if img->y0 is odd, then first line shall use Cb/Cr = 0 */
    offy = img->y0 & 1U;
    loopmaxh = maxh - offy;

    if (offy > 0U) {
        size_t j;

        for (j = 0; j < maxw; ++j) {
            sycc_to_rgb(offset, upb, *y, 0, 0, r, g, b);
            ++y;
            ++r;
            ++g;
            ++b;
        }
    }

    for (i = 0U; i < (loopmaxh & ~(size_t)1U); i += 2U) {
        size_t j;

        ny = y + maxw;
        nr = r + maxw;
        ng = g + maxw;
        nb = b + maxw;

        if (offx > 0U) {
            sycc_to_rgb(offset, upb, *y, 0, 0, r, g, b);
            ++y;
            ++r;
            ++g;
            ++b;
            sycc_to_rgb(offset, upb, *ny, *cb, *cr, nr, ng, nb);
            ++ny;
            ++nr;
            ++ng;
            ++nb;
        }

        for (j = 0; j < (loopmaxw & ~(size_t)1U); j += 2U) {
            sycc_to_rgb(offset, upb, *y, *cb, *cr, r, g, b);
            ++y;
            ++r;
            ++g;
            ++b;
            sycc_to_rgb(offset, upb, *y, *cb, *cr, r, g, b);
            ++y;
            ++r;
            ++g;
            ++b;

            sycc_to_rgb(offset, upb, *ny, *cb, *cr, nr, ng, nb);
            ++ny;
            ++nr;
            ++ng;
            ++nb;
            sycc_to_rgb(offset, upb, *ny, *cb, *cr, nr, ng, nb);
            ++ny;
            ++nr;
            ++ng;
            ++nb;
            ++cb;
            ++cr;
        }
        if (j < loopmaxw) {
            sycc_to_rgb(offset, upb, *y, *cb, *cr, r, g, b);
            ++y;
            ++r;
            ++g;
            ++b;

            sycc_to_rgb(offset, upb, *ny, *cb, *cr, nr, ng, nb);
            ++ny;
            ++nr;
            ++ng;
            ++nb;
            ++cb;
            ++cr;
        }
        y += maxw;
        r += maxw;
        g += maxw;
        b += maxw;
    }
    if (i < loopmaxh) {
        size_t j;

        for (j = 0U; j < (maxw & ~(size_t)1U); j += 2U) {
            sycc_to_rgb(offset, upb, *y, *cb, *cr, r, g, b);

            ++y;
            ++r;
            ++g;
            ++b;

            sycc_to_rgb(offset, upb, *y, *cb, *cr, r, g, b);

            ++y;
            ++r;
            ++g;
            ++b;
            ++cb;
            ++cr;
        }
        if (j < maxw) {
            sycc_to_rgb(offset, upb, *y, *cb, *cr, r, g, b);
        }
    }

    opj_image_data_free(img->comps[0].data);
    img->comps[0].data = d0;
    opj_image_data_free(img->comps[1].data);
    img->comps[1].data = d1;
    opj_image_data_free(img->comps[2].data);
    img->comps[2].data = d2;

    img->comps[1].w = img->comps[2].w = img->comps[0].w;
    img->comps[1].h = img->comps[2].h = img->comps[0].h;
    img->comps[1].dx = img->comps[2].dx = img->comps[0].dx;
    img->comps[1].dy = img->comps[2].dy = img->comps[0].dy;
    img->color_space = OPJ_CLRSPC_SRGB;
    return;

fails:
    opj_image_data_free(r);
    opj_image_data_free(g);
    opj_image_data_free(b);
} /* sycc420_to_rgb() */

static void color_sycc_to_rgb(opj_image_t *img) {
    if (img->numcomps < 3) {
        img->color_space = OPJ_CLRSPC_GRAY;
        return;
    }

    if ((img->comps[0].dx == 1) && (img->comps[1].dx == 2) && (img->comps[2].dx == 2) && (img->comps[0].dy == 1) && (img->comps[1].dy == 2) &&
        (img->comps[2].dy == 2)) { /* horizontal and vertical sub-sample */
        sycc420_to_rgb(img);
    } else if ((img->comps[0].dx == 1) && (img->comps[1].dx == 2) && (img->comps[2].dx == 2) && (img->comps[0].dy == 1) && (img->comps[1].dy == 1) &&
               (img->comps[2].dy == 1)) { /* horizontal sub-sample only */
        sycc422_to_rgb(img);
    } else if ((img->comps[0].dx == 1) && (img->comps[1].dx == 1) && (img->comps[2].dx == 1) && (img->comps[0].dy == 1) && (img->comps[1].dy == 1) &&
               (img->comps[2].dy == 1)) { /* no sub-sample */
        sycc444_to_rgb(img);
    } else {
        DEBUGF("%s:%d:color_sycc_to_rgb\n\tCAN NOT CONVERT\n", __FILE__, __LINE__);
        return;
    }
} /* color_sycc_to_rgb() */

static void color_cmyk_to_rgb(opj_image_t *image) {
    float C, M, Y, K;
    float sC, sM, sY, sK;
    unsigned int w, h, max, i;

    w = image->comps[0].w;
    h = image->comps[0].h;

    if ((image->numcomps < 4) || (image->comps[0].dx != image->comps[1].dx) || (image->comps[0].dx != image->comps[2].dx) || (image->comps[0].dx != image->comps[3].dx) ||
        (image->comps[0].dy != image->comps[1].dy) || (image->comps[0].dy != image->comps[2].dy) || (image->comps[0].dy != image->comps[3].dy)) {
        DEBUGF("%s:%d:color_cmyk_to_rgb\n\tCAN NOT CONVERT\n", __FILE__, __LINE__);
        return;
    }

    max = w * h;

    sC = 1.0F / (float)((1 << image->comps[0].prec) - 1);
    sM = 1.0F / (float)((1 << image->comps[1].prec) - 1);
    sY = 1.0F / (float)((1 << image->comps[2].prec) - 1);
    sK = 1.0F / (float)((1 << image->comps[3].prec) - 1);

    for (i = 0; i < max; ++i) {
        /* CMYK values from 0 to 1 */
        C = (float)(image->comps[0].data[i]) * sC;
        M = (float)(image->comps[1].data[i]) * sM;
        Y = (float)(image->comps[2].data[i]) * sY;
        K = (float)(image->comps[3].data[i]) * sK;

        /* Invert all CMYK values */
        C = 1.0F - C;
        M = 1.0F - M;
        Y = 1.0F - Y;
        K = 1.0F - K;

        /* CMYK -> RGB : RGB results from 0 to 255 */
        image->comps[0].data[i] = (int)(255.0F * C * K); /* R */
        image->comps[1].data[i] = (int)(255.0F * M * K); /* G */
        image->comps[2].data[i] = (int)(255.0F * Y * K); /* B */
    }

    opj_image_data_free(image->comps[3].data);
    image->comps[3].data = NULL;
    image->comps[0].prec = 8;
    image->comps[1].prec = 8;
    image->comps[2].prec = 8;
    image->numcomps -= 1;
    image->color_space = OPJ_CLRSPC_SRGB;

    for (i = 3; i < image->numcomps; ++i) {
        memcpy(&(image->comps[i]), &(image->comps[i + 1]), sizeof(image->comps[i]));
    }

} /* color_cmyk_to_rgb() */

/*
 * This code has been adopted from sjpx_openjpeg.c of ghostscript
 */
static void color_esycc_to_rgb(opj_image_t *image) {
    int y, cb, cr, sign1, sign2, val;
    unsigned int w, h, max, i;
    int flip_value = (1 << (image->comps[0].prec - 1));
    int max_value = (1 << image->comps[0].prec) - 1;

    if ((image->numcomps < 3) || (image->comps[0].dx != image->comps[1].dx) || (image->comps[0].dx != image->comps[2].dx) || (image->comps[0].dy != image->comps[1].dy) ||
        (image->comps[0].dy != image->comps[2].dy)) {
        DEBUGF("%s:%d:color_esycc_to_rgb\n\tCAN NOT CONVERT\n", __FILE__, __LINE__);
        return;
    }

    w = image->comps[0].w;
    h = image->comps[0].h;

    sign1 = (int)image->comps[1].sgnd;
    sign2 = (int)image->comps[2].sgnd;

    max = w * h;

    for (i = 0; i < max; ++i) {
        y = image->comps[0].data[i];
        cb = image->comps[1].data[i];
        cr = image->comps[2].data[i];

        if (!sign1) {
            cb -= flip_value;
        }
        if (!sign2) {
            cr -= flip_value;
        }

        val = (int)((float)y - (float)0.0000368 * (float)cb + (float)1.40199 * (float)cr + (float)0.5);

        if (val > max_value) {
            val = max_value;
        } else if (val < 0) {
            val = 0;
        }
        image->comps[0].data[i] = val;

        val = (int)((float)1.0003 * (float)y - (float)0.344125 * (float)cb - (float)0.7141128 * (float)cr + (float)0.5);

        if (val > max_value) {
            val = max_value;
        } else if (val < 0) {
            val = 0;
        }
        image->comps[1].data[i] = val;

        val = (int)((float)0.999823 * (float)y + (float)1.77204 * (float)cb - (float)0.000008 * (float)cr + (float)0.5);

        if (val > max_value) {
            val = max_value;
        } else if (val < 0) {
            val = 0;
        }
        image->comps[2].data[i] = val;
    }
    image->color_space = OPJ_CLRSPC_SRGB;

} /* color_esycc_to_rgb() */

static opj_image_t *convert_gray_to_rgb(opj_image_t *original) {
    OPJ_UINT32 compno;
    opj_image_t *l_new_image = NULL;
    opj_image_cmptparm_t *l_new_components = NULL;

    l_new_components = (opj_image_cmptparm_t *)malloc((original->numcomps + 2U) * sizeof(opj_image_cmptparm_t));
    if (l_new_components == NULL) {
        DEBUGF("ERROR -> opj_decompress: failed to allocate memory for RGB image!\n");
        opj_image_destroy(original);
        return NULL;
    }

    l_new_components[0].dx = l_new_components[1].dx = l_new_components[2].dx = original->comps[0].dx;
    l_new_components[0].dy = l_new_components[1].dy = l_new_components[2].dy = original->comps[0].dy;
    l_new_components[0].h = l_new_components[1].h = l_new_components[2].h = original->comps[0].h;
    l_new_components[0].w = l_new_components[1].w = l_new_components[2].w = original->comps[0].w;
    l_new_components[0].prec = l_new_components[1].prec = l_new_components[2].prec = original->comps[0].prec;
    l_new_components[0].sgnd = l_new_components[1].sgnd = l_new_components[2].sgnd = original->comps[0].sgnd;
    l_new_components[0].x0 = l_new_components[1].x0 = l_new_components[2].x0 = original->comps[0].x0;
    l_new_components[0].y0 = l_new_components[1].y0 = l_new_components[2].y0 = original->comps[0].y0;

    for (compno = 1U; compno < original->numcomps; ++compno) {
        l_new_components[compno + 2U].dx = original->comps[compno].dx;
        l_new_components[compno + 2U].dy = original->comps[compno].dy;
        l_new_components[compno + 2U].h = original->comps[compno].h;
        l_new_components[compno + 2U].w = original->comps[compno].w;
        l_new_components[compno + 2U].prec = original->comps[compno].prec;
        l_new_components[compno + 2U].sgnd = original->comps[compno].sgnd;
        l_new_components[compno + 2U].x0 = original->comps[compno].x0;
        l_new_components[compno + 2U].y0 = original->comps[compno].y0;
    }

    l_new_image = opj_image_create(original->numcomps + 2U, l_new_components, OPJ_CLRSPC_SRGB);
    free(l_new_components);
    if (l_new_image == NULL) {
        DEBUGF("ERROR -> opj_decompress: failed to allocate memory for RGB image!\n");
        opj_image_destroy(original);
        return NULL;
    }

    l_new_image->x0 = original->x0;
    l_new_image->x1 = original->x1;
    l_new_image->y0 = original->y0;
    l_new_image->y1 = original->y1;

    l_new_image->comps[0].factor = l_new_image->comps[1].factor = l_new_image->comps[2].factor = original->comps[0].factor;
    l_new_image->comps[0].alpha = l_new_image->comps[1].alpha = l_new_image->comps[2].alpha = original->comps[0].alpha;
    l_new_image->comps[0].resno_decoded = l_new_image->comps[1].resno_decoded = l_new_image->comps[2].resno_decoded = original->comps[0].resno_decoded;

    memcpy(l_new_image->comps[0].data, original->comps[0].data, sizeof(OPJ_INT32) * original->comps[0].w * original->comps[0].h);
    memcpy(l_new_image->comps[1].data, original->comps[0].data, sizeof(OPJ_INT32) * original->comps[0].w * original->comps[0].h);
    memcpy(l_new_image->comps[2].data, original->comps[0].data, sizeof(OPJ_INT32) * original->comps[0].w * original->comps[0].h);

    for (compno = 1U; compno < original->numcomps; ++compno) {
        l_new_image->comps[compno + 2U].factor = original->comps[compno].factor;
        l_new_image->comps[compno + 2U].alpha = original->comps[compno].alpha;
        l_new_image->comps[compno + 2U].resno_decoded = original->comps[compno].resno_decoded;
        memcpy(l_new_image->comps[compno + 2U].data, original->comps[compno].data, sizeof(OPJ_INT32) * original->comps[compno].w * original->comps[compno].h);
    }
    opj_image_destroy(original);
    return l_new_image;
}

/* Component precision scaling */
static void scale_component_up(opj_image_comp_t *component, OPJ_UINT32 precision) {
    OPJ_SIZE_T i, len;

    len = (OPJ_SIZE_T)component->w * (OPJ_SIZE_T)component->h;
    if (component->sgnd) {
        OPJ_INT64 newMax = (OPJ_INT64)(1U << (precision - 1));
        OPJ_INT64 oldMax = (OPJ_INT64)(1U << (component->prec - 1));
        OPJ_INT32 *l_data = component->data;
        for (i = 0; i < len; ++i) {
            l_data[i] = (OPJ_INT32)(((OPJ_INT64)l_data[i] * newMax) / oldMax);
        }
    } else {
        OPJ_UINT64 newMax = (OPJ_UINT64)((1U << precision) - 1U);
        OPJ_UINT64 oldMax = (OPJ_UINT64)((1U << component->prec) - 1U);
        OPJ_UINT32 *l_data = (OPJ_UINT32 *)component->data;
        for (i = 0; i < len; ++i) {
            l_data[i] = (OPJ_UINT32)(((OPJ_UINT64)l_data[i] * newMax) / oldMax);
        }
    }
    component->prec = precision;
}

static void scale_component(opj_image_comp_t *component, OPJ_UINT32 precision) {
    int shift;
    OPJ_SIZE_T i, len;

    if (component->prec == precision) {
        return;
    }
    if (component->prec < precision) {
        scale_component_up(component, precision);
        return;
    }
    shift = (int)(component->prec - precision);
    len = (OPJ_SIZE_T)component->w * (OPJ_SIZE_T)component->h;
    if (component->sgnd) {
        OPJ_INT32 *l_data = component->data;
        for (i = 0; i < len; ++i) {
            l_data[i] >>= shift;
        }
    } else {
        OPJ_UINT32 *l_data = (OPJ_UINT32 *)component->data;
        for (i = 0; i < len; ++i) {
            l_data[i] >>= shift;
        }
    }
    component->prec = precision;
}

BITMAP *load_jp2(AL_CONST char *filename, RGB *pal) {
    opj_stream_t *l_stream = opj_stream_create_default_file_stream(filename, true);
    if (!l_stream) {
        DEBUGF("ERROR -> failed to create the stream from the file %s\n", filename);
        return NULL;
    }

    opj_codec_t *l_codec = opj_create_decompress(OPJ_CODEC_JP2);

    opj_set_info_handler(l_codec, info_callback, 0);
    opj_set_warning_handler(l_codec, warning_callback, 0);
    opj_set_error_handler(l_codec, error_callback, 0);

    /* Setup the decoder decoding parameters */
    opj_dparameters_t params;
    opj_set_default_decoder_parameters(&params);
    if (!opj_setup_decoder(l_codec, &params)) {
        DEBUGF("ERROR -> opj_decompress: failed to setup the decoder\n");
        opj_stream_destroy(l_stream);
        opj_destroy_codec(l_codec);
        return NULL;
    }

    /* Read the main header of the codestream and if necessary the JP2 boxes*/
    opj_image_t *image = NULL;
    if (!opj_read_header(l_stream, l_codec, &image)) {
        DEBUGF("ERROR -> opj_decompress: failed to read the header\n");
        opj_stream_destroy(l_stream);
        opj_destroy_codec(l_codec);
        opj_image_destroy(image);
        return NULL;
    }

    /* Get the decoded image */
    if (!(opj_decode(l_codec, l_stream, image) && opj_end_decompress(l_codec, l_stream))) {
        DEBUGF("ERROR -> opj_decompress: failed to decode image!\n");
        opj_destroy_codec(l_codec);
        opj_stream_destroy(l_stream);
        opj_image_destroy(image);
        return NULL;
    }

    /* Close the byte stream */
    opj_stream_destroy(l_stream);

    if (image->color_space != OPJ_CLRSPC_SYCC && image->numcomps == 3 && image->comps[0].dx == image->comps[0].dy && image->comps[1].dx != 1) {
        image->color_space = OPJ_CLRSPC_SYCC;
    } else if (image->numcomps <= 2) {
        image->color_space = OPJ_CLRSPC_GRAY;
    }

    if (image->color_space == OPJ_CLRSPC_SYCC) {
        color_sycc_to_rgb(image);
    } else if (image->color_space == OPJ_CLRSPC_CMYK) {
        color_cmyk_to_rgb(image);
    } else if (image->color_space == OPJ_CLRSPC_EYCC) {
        color_esycc_to_rgb(image);
    }

    for (OPJ_UINT32 compno = 0; compno < image->numcomps; ++compno) {
        scale_component(&(image->comps[compno]), 8);
    }

    switch (image->color_space) {
        case OPJ_CLRSPC_SRGB:
            break;
        case OPJ_CLRSPC_GRAY:
            image = convert_gray_to_rgb(image);
            break;
        default:
            DEBUGF("ERROR -> opj_decompress: don't know how to convert image to RGB colorspace!\n");
            opj_image_destroy(image);
            image = NULL;
            break;
    }
    if (image == NULL) {
        DEBUGF("ERROR -> opj_decompress: failed to convert to RGB image!\n");
        opj_destroy_codec(l_codec);
        return NULL;
    }

    /* free image data structure */
    opj_image_destroy(image);

    return NULL;
}

#endif  // USE_JASPER
