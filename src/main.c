#include <conio.h>
#include <stdarg.h>

#include "main.h"

#include "alpng.h"
#include "format-qoi.h"
#include "format-webp.h"
#include "format-jpeg.h"

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

#define FORMATS_READ "BMP, PCX, TGA, LBM, QOI, JPG, PNG, WEB"
#define FORMATS_WRITE "BMP, PCX, TGA, QOI, JPG, PNG, WEB"

int output_quality = 95;

typedef struct {
    int width;
    int height;
} gfx_mode_t;

static char *lastError;

static gfx_mode_t all_modes[] = {{.width = 320, .height = 240},   // 320x240
                                 {.width = 640, .height = 480},   // 640x480
                                 {.width = 800, .height = 600},   // 800x600
                                 {.width = 1024, .height = 768},  // 1024x768
                                 {.width = 1280, .height = 960},  // 1280x960
                                 {.width = 0, .height = 0}};

static void usage() {
    fputs("Usage:\n", stderr);
    fputs("  DOSVIEW.EXE [-hbl] [-q <quality>] [-w <width>] [-s <outfile>] <infile>\n", stderr);
    fputs("  -h           : show this screen\n", stderr);
    fputs("  -l           : list know screen modes\n", stderr);
    fputs("  -b           : try 32bpp screen mode instead of 24bpp\n", stderr);
    fputs("  -w <width>   : screen width to use.\n", stderr);
    fputs("  -s <outfile> : do not show the image, save it to outfile instead.\n", stderr);
    fputs("  -q <quality> : Quality for writing JPEG and WEPB image (1..100).\n", stderr);
    fputs("\n", stderr);
    fputs("Input formats  : " FORMATS_READ " \n", stderr);
    fputs("Output formats : " FORMATS_WRITE " \n", stderr);
    fputs("\n", stderr);
    fputs("This is DosView 1.0\n", stderr);
    fputs("(c) 2023 by Andre Seidelt <superilu@yahoo.com> and others.\n", stderr);
    fputs("See LICENSE for detailed licensing information.\n", stderr);
    fputs("\n", stderr);
    exit(EXIT_FAILURE);
}

/**
 * @brief initialize last error function.
 */
static void init_last_error() { lastError = NULL; }

/**
 * @brief free any memnory of the last error.
 */
static void clear_last_error() {
    if (lastError) {
        free(lastError);
        lastError = NULL;
    }
}

/**
 * @brief Set the last error. the string data is copied to malloc()ed RAM.
 *
 * @param err the last error string.
 */
static void set_last_error(const char *err, ...) {
    va_list args;

    clear_last_error();
    size_t mem_size = strlen(err) * 4;  // hack, uhm, heuristics :)
    lastError = calloc(1, mem_size);
    if (lastError) {
        va_start(args, err);
        vsnprintf(lastError, mem_size, err, args);
        va_end(args);
    }
}

static void list_modes() {
    gfx_mode_t *m = &all_modes[0];

    fprintf(stderr, "Known modes:\n");
    while (m->width) {
        fprintf(stderr, "  %dx%d\n", m->width, m->height);
        m++;
    }
    exit(EXIT_FAILURE);
}

static void register_formats() {
    alpng_init();
    register_bitmap_file_type("qoi", load_qoi, save_qoi, NULL);
    register_bitmap_file_type("web", load_webp, save_webp, NULL);
    register_bitmap_file_type("jpg", load_jpeg, save_jpeg, NULL);
}

static void clean_exit(int code) {
    allegro_exit();
    textmode(C80);
    if (lastError) {
        fputs(lastError, stdout);
        fputs("\nERROR\n", stdout);
    } else {
        fputs("OK\n", stdout);
    }
    exit(code);
}

int main(int argc, char *argv[]) {
    int opt = 0;
    char *fname = NULL;
    char *outfile = NULL;
    int screen_width = 640;
    int screen_height = 0;
    bool bit32 = false;
    int x_start = 0;
    int y_start = 0;
    int scaled_height;
    int scaled_width;
    float factor = 1.0f;  // 1.0 is 'fit on screen'

    while ((opt = getopt(argc, argv, "lbhw:s:q:")) != -1) {
        switch (opt) {
            case 'w':
                screen_width = atoi(optarg);
                break;
            case 'q':
                output_quality = atoi(optarg);
                break;
            case 's':
                outfile = optarg;
                break;
            case 'b':
                bit32 = true;
                break;
            case 'l':
                list_modes();
                break;
            case 'h':
            default: /* '?' */
                usage();
                break;
        }
    }

    if (optind < argc) {
        fname = argv[optind];
    } else {
        usage();
    }

    if (output_quality < 1 || output_quality > 100) {
        usage();
    }

    gfx_mode_t *m = &all_modes[0];
    while (m->width) {
        if (screen_width == m->width) {
            screen_height = m->height;
            break;
        } else {
            m++;
        }
    }

    if (!screen_height) {
        fprintf(stderr, "Unknown screen width %d\n", screen_width);
        list_modes();
    }

    init_last_error();
    allegro_init();
    install_keyboard();
    register_formats();

    if (bit32) {
        set_color_depth(32);
    } else {
        set_color_depth(24);
    }

    if (set_gfx_mode(GFX_AUTODETECT, screen_width, screen_height, 0, 0) != 0) {
        set_last_error("Couldn't set a %d bit color resolution at %dx%d: %s\n", bit32 ? 32 : 24, screen_width, screen_height, allegro_error);
        clean_exit(EXIT_SUCCESS);
    }

    BITMAP *bm = load_bitmap(fname, NULL);
    if (!bm) {
        set_last_error("Can't load image %s", fname);
        clean_exit(EXIT_SUCCESS);
    }

    DEBUGF("image size = %dx%d\n", bm->w, bm->h);

    // scale to "fit screen"
    factor = (float)bm->w / (float)screen_width;
    scaled_width = screen_width * factor;
    scaled_height = screen_height * factor;
    if (scaled_width >= bm->w) {
        factor = (float)bm->w / (float)screen_width;
    }
    if (scaled_height >= bm->h) {
        factor = (float)bm->h / (float)screen_height;
    }
    scaled_width = screen_width * factor;
    scaled_height = screen_height * factor;

    if (!outfile) {
        BITMAP *tmp = create_bitmap_ex(get_color_depth(), bm->w, bm->h);
        while (true) {
            //////
            /// draw image
            clear_to_color(screen, 0);

            DEBUGF("stretch_blit(%d, %d, %d, %d, %d, %d, %d, %d)\n", x_start, y_start, scaled_width, scaled_height, 0, 0, screen_width, screen_height);
            blit(bm, tmp, 0, 0, 0, 0, bm->w, bm->h);
            stretch_blit(tmp, screen, x_start, y_start, scaled_width, scaled_height, 0, 0, screen_width, screen_height);

            //////
            /// handle input
            if (keyboard_needs_poll()) {
                poll_keyboard();
            }
            int key = readkey();
            int key_upper = (key >> 8);
            char key_lower = (char)(key & 0xFF);

            DEBUGF("key=%04X\n", key);

            // modifiers
            int stepsize = 1;
            float scale_step = 0.1f;
            if (key_shifts & KB_SHIFT_FLAG) {
                stepsize *= 4;
                scale_step *= 0.2f;
            }
            if (key_shifts & KB_CTRL_FLAG) {
                stepsize *= 8;
                scale_step *= 0.4f;
            }
            if (key_shifts & KB_ALT_FLAG) {
                stepsize *= 16;
                scale_step *= 0.8f;
            }

            // keys
            if (key_upper == KEY_ESC) {
                break;  // exit
            } else if (key_upper == KEY_LEFT) {
                if (x_start > 0) {
                    x_start -= stepsize;
                }
            } else if (key_upper == KEY_RIGHT) {
                if (x_start + screen_width < bm->w) {
                    x_start += stepsize;
                }
            } else if (key_upper == KEY_UP) {
                if (y_start > 0) {
                    y_start -= stepsize;
                }
            } else if (key_upper == KEY_DOWN) {
                if (y_start + screen_height < bm->h) {
                    y_start += stepsize;
                }
            } else if (key_upper == KEY_PGDN) {
                factor += scale_step;
            } else if (key_upper == KEY_PGUP) {
                factor -= scale_step;
            } else if ((key_lower == 'F') || (key_lower == 'f')) {
                factor = (float)bm->w / (float)screen_width;  // full zoom
            } else if ((key_lower == 'Z') || (key_lower == 'z')) {
                factor = 1.0f;  // fit on screen
            } else if ((key_lower == 'Q') || (key_lower == 'q')) {
                break;  // exit
            }

            //////
            // sanitychecks
            scaled_width = screen_width * factor;
            scaled_height = screen_height * factor;

            if (scaled_width >= bm->w) {
                factor = (float)bm->w / (float)screen_width;
            }
            if (scaled_height >= bm->h) {
                factor = (float)bm->h / (float)screen_height;
            }
            if (factor < 0.0f) {
                factor = 0.0f;
            }

            scaled_width = screen_width * factor;
            scaled_height = screen_height * factor;

            if (x_start + scaled_width >= bm->w) {
                x_start = bm->w - scaled_width - 1;
            }
            if (y_start + scaled_height >= bm->h) {
                y_start = bm->h - scaled_height - 1;
            }
            if (x_start < 0) {
                x_start = 0;
            }
            if (y_start < 0) {
                y_start = 0;
            }

            DEBUGF("start = %dx%d, stepsize = %d, factor=%f, scaled=%dx%d\n", x_start, y_start, stepsize, factor, scaled_width, scaled_height);
        }
        destroy_bitmap(tmp);
    } else {
        if (save_bitmap(outfile, bm, NULL)) {
            set_last_error("Can't save image %s", outfile);
            clean_exit(EXIT_SUCCESS);
        }
    }
    destroy_bitmap(bm);

    clean_exit(EXIT_SUCCESS);
}
