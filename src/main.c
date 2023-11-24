#include <conio.h>
#include <stdarg.h>

#include "main.h"

#include "alpng.h"
#include "algif.h"
#include "format-qoi.h"
#include "format-webp.h"
#include "format-jpeg.h"
#include "format-tiff.h"
#include "format-jasper.h"
#include "format-stb.h"

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

#define DEFAULT_MODE 1

#define MIN_ZOOM 100

#define FORMATS_READ "BMP, PCX, TGA, LBM, QOI, JPG, PNG, WEB, TIF, JP2\n                 GIF, PSD, HDR, PIC"
#define FORMATS_WRITE "BMP, PCX, TGA, QOI, JPG, PNG, WEB, TIF, JP2, GIF"

int output_quality = 95;

typedef struct {
    int width;
    int height;
    int available;
} gfx_mode_t;

static char *lastError;

static gfx_mode_t all_modes[] = {{.width = 320, .height = 240, .available = 0},    // 320x240
                                 {.width = 640, .height = 480, .available = 0},    // 640x480
                                 {.width = 800, .height = 600, .available = 0},    // 800x600
                                 {.width = 1024, .height = 768, .available = 0},   // 1024x768
                                 {.width = 1280, .height = 960, .available = 0},   // 1280x960
                                 {.width = 1280, .height = 1024, .available = 0},  // 1280x1024
                                 {.width = 1600, .height = 1200, .available = 0},  // 1600x1200
                                 {.width = 1920, .height = 1080, .available = 0},  // 1920x1080
                                 {.width = 0, .height = 0}};

static void banner(FILE *out) {
    fputs("This is DosView 1.4 (https://github.com/SuperIlu/DosView)\n", out);
    fputs("(c) 2023 by Andre Seidelt <superilu@yahoo.com> and others.\n", out);
    fputs("See LICENSE for detailed licensing information.\n", out);
    fputs("\n", out);
}

static void usage() {
    banner(stderr);
    fputs("Usage:\n", stderr);
    fputs("  DOSVIEW.EXE [-hkl] [-q <quality>] [-r <num>] [-s <outfile>] <infile>\n", stderr);
    fputs("  -h           : show this screen.\n", stderr);
    fputs("  -k           : keys help.\n", stderr);
    fputs("  -l           : list know screen modes.\n", stderr);
    fputs("  -r <num>     : screen mode to use (use -l for a list).\n", stderr);
    fputs("  -s <outfile> : do not show the image, save it to outfile instead.\n", stderr);
    fputs("  -f <factor>  : scale saved image, <1 reduce, >1 enlarge (float).\n", stderr);
    fputs("  -q <quality> : Quality for writing JPG/WEP/JP2 image (1..100). Default: 95\n", stderr);
    fputs("\n", stderr);
    fputs("Input formats  : " FORMATS_READ " \n", stderr);
    fputs("Output formats : " FORMATS_WRITE " \n", stderr);
    fputs("\n", stderr);
    exit(EXIT_FAILURE);
}

static void keys() {
    banner(stderr);
    fputs("Keys:\n", stderr);
    fputs("  - `ESC`/`Q`       : quit\n", stderr);
    fputs("  - `F`             : show actual size\n", stderr);
    fputs("  - `Z`             : fit to screen\n", stderr);
    fputs("  - `I`             : toggle image info\n", stderr);
    fputs("  - `PAGE UP`/`9`   : increase zoom\n", stderr);
    fputs("  - `PAGE DOWN`/`3` : decrease zoom\n", stderr);
    fputs("  - `UP`/`8`        : move image up\n", stderr);
    fputs("  - `DOWN`/`2`      : move image down\n", stderr);
    fputs("  - `LEFT`/`4`      : move image left\n", stderr);
    fputs("  - `RIGHT`/`6`     : move image right\n", stderr);
    fputs("  - `SHIFT`         : move/scale 2x as fast\n", stderr);
    fputs("  - `CTRL`          : move/scale 4x as fast\n", stderr);
    fputs("  - `ALT`           : move/scale 8x as fast\n", stderr);
    fputs("\n", stderr);
    fputs("  `SHIFT`, `ALT` and `CTRL` can be used in any combination.\n", stderr);
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
    // check available modes
    init_last_error();
    allegro_init();
    for (int i = 0; all_modes[i].width; i++) {
        set_color_depth(32);
        if (set_gfx_mode(GFX_AUTODETECT, all_modes[i].width, all_modes[i].height, 0, 0) != 0) {
            set_color_depth(24);
            if (set_gfx_mode(GFX_AUTODETECT, all_modes[i].width, all_modes[i].height, 0, 0) != 0) {
                set_color_depth(8);
                if (set_gfx_mode(GFX_AUTODETECT, all_modes[i].width, all_modes[i].height, 0, 0) != 0) {
                    all_modes[i].available = 0;
                } else {
                    all_modes[i].available = get_color_depth();
                }
            } else {
                all_modes[i].available = get_color_depth();
            }
        } else {
            all_modes[i].available = get_color_depth();
        }
    }
    allegro_exit();
    textmode(C80);

    // print available modes
    banner(stdout);
    fprintf(stdout, "Display modes:\n");
    for (int i = 0; all_modes[i].width; i++) {
        if (all_modes[i].available) {
            fprintf(stdout, "  #% 2d: %4dx%4d := %dbpp %s\n", i, all_modes[i].width, all_modes[i].height, all_modes[i].available, i == DEFAULT_MODE ? "(default)" : "");
        } else {
            fprintf(stdout, "  #% 2d: %4dx%4d := UNAVAILABLE %s\n", i, all_modes[i].width, all_modes[i].height, i == DEFAULT_MODE ? "(default)" : "");
        }
    }
    exit(EXIT_FAILURE);
}

static void register_formats() {
    alpng_init();
    algif_init();
    register_bitmap_file_type("qoi", load_qoi, save_qoi, NULL);
    register_bitmap_file_type("web", load_webp, save_webp, NULL);
    register_bitmap_file_type("jpg", load_jpeg, save_jpeg, NULL);
    register_bitmap_file_type("tif", load_tiff, save_tiff, NULL);
    register_bitmap_file_type("jp2", load_jasper, save_jasper, NULL);
    register_bitmap_file_type("ras", load_jasper, save_jasper, NULL);
    register_bitmap_file_type("psd", load_stb, NULL, NULL);
    register_bitmap_file_type("hdr", load_stb, NULL, NULL);
    register_bitmap_file_type("pic", load_stb, NULL, NULL);

    // add all netpbm formats
    register_bitmap_file_type("pnm", load_jasper, save_jasper, NULL);
    register_bitmap_file_type("pbm", load_jasper, save_jasper, NULL);
    register_bitmap_file_type("pgm", load_jasper, save_jasper, NULL);
    register_bitmap_file_type("ppm", load_jasper, save_jasper, NULL);
}

static void clean_exit(int code) {
    allegro_exit();
    // textmode(C80);
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
    char *infile = NULL;
    char *outfile = NULL;
    int mode = DEFAULT_MODE;
    int screen_width = 0;
    int screen_height = 0;
    int x_start = 0;
    int y_start = 0;
    int scaled_height;
    int scaled_width;
    float factor = 1.0f;  // 1.0 is 'fit on screen'
    bool image_info = false;
    float scale = 1.0f;

    while ((opt = getopt(argc, argv, "klhr:s:q:f:")) != -1) {
        switch (opt) {
            case 'r':
                mode = atoi(optarg);
                break;
            case 'q':
                output_quality = atoi(optarg);
                break;
            case 'f':
                scale = atof(optarg);
                break;
            case 's':
                outfile = optarg;
                break;
            case 'l':
                list_modes();
                break;
            case 'k':
                keys();
                break;
            case 'h':
            default: /* '?' */
                usage();
                break;
        }
    }

    if (optind < argc) {
        infile = argv[optind];
    } else {
        usage();
    }

    if (output_quality < 1 || output_quality > 100) {
        usage();
    }

    for (int i = 0; all_modes[i].width; i++) {
        if (i == mode) {
            screen_width = all_modes[i].width;
            screen_height = all_modes[i].height;
        }
    }

    if (!screen_width || !screen_height) {
        fprintf(stderr, "Unknown screen mode %d\n", mode);
        list_modes();
    }

    init_last_error();
    allegro_init();
    register_formats();
    install_keyboard();
    set_color_conversion(COLORCONV_TOTAL);

    set_color_depth(32);
    // if (!outfile) {
    if (set_gfx_mode(GFX_AUTODETECT, screen_width, screen_height, 0, 0) != 0) {
        set_color_depth(24);
        if (set_gfx_mode(GFX_AUTODETECT, screen_width, screen_height, 0, 0) != 0) {
            set_color_depth(8);
            if (set_gfx_mode(GFX_AUTODETECT, screen_width, screen_height, 0, 0) != 0) {
                set_last_error("Resolution %dx%d not available: %s\n", screen_width, screen_height, allegro_error);
                clean_exit(EXIT_SUCCESS);
            }
        }
    }
    DEBUGF("%dx%d at %dbpp\n", screen_width, screen_height, get_color_depth());
    // } else {
    //     _rgb_r_shift_32 = 16;
    //     _rgb_g_shift_32 = 8;
    //     _rgb_b_shift_32 = 0;
    //     _rgb_a_shift_32 = 24;
    // }
    // printf("_rgb_r_shift_32=%d", _rgb_r_shift_32);
    // printf("_rgb_g_shift_32=%d", _rgb_g_shift_32);
    // printf("_rgb_b_shift_32=%d", _rgb_b_shift_32);
    // printf("_rgb_a_shift_32=%d", _rgb_a_shift_32);

    PALETTE pal;
    BITMAP *bm = load_bitmap(infile, pal);
    if (!bm) {
        set_last_error("Can't load image %s", infile);
        clean_exit(EXIT_SUCCESS);
    }

    DEBUGF("image size = %dx%d @ %dbpp\n", bm->w, bm->h, bitmap_color_depth(bm));

    if (!outfile) {
        if (get_color_depth() == 8) {
            set_palette(pal);
        }

        // convert image to display color depth
        BITMAP *tmp = create_bitmap_ex(get_color_depth(), bm->w, bm->h);
        blit(bm, tmp, 0, 0, 0, 0, bm->w, bm->h);
        destroy_bitmap(bm);

        // scale to "fit screen" factor
        if (tmp->w > tmp->h) {
            factor = (float)screen_width / (float)tmp->w;
        } else {
            factor = (float)screen_height / (float)tmp->h;
        }

        // calculate the scaled size of the image
        scaled_width = tmp->w * factor;
        scaled_height = tmp->h * factor;

        while (true) {
            //////
            /// draw image
            clear_to_color(screen, 0);

            DEBUGF("start = %dx%d, factor=%f, scaled=%dx%d\n", x_start, y_start, factor, scaled_width, scaled_height);
            int src_x, src_y, src_w, src_h, dest_x, dest_y, dest_w, dest_h;

            if (scaled_width <= screen_width) {
                DEBUG("W1\n");
                src_x = 0;
                src_w = tmp->w;
                dest_x = (screen_width / 2 - scaled_width / 2);
                dest_w = scaled_width;
            } else {
                DEBUG("W2\n");
                src_x = x_start;
                src_w = (tmp->w * screen_width) / scaled_width;
                dest_x = 0;
                dest_w = screen_width;
            }

            if (scaled_height <= screen_height) {
                DEBUG("H1\n");
                src_y = 0;
                src_h = tmp->h;
                dest_y = (screen_height / 2 - scaled_height / 2);
                dest_h = scaled_height;
            } else {
                DEBUG("H2\n");
                src_y = y_start;
                src_h = (tmp->h * screen_height) / scaled_height;
                dest_y = 0;
                dest_h = screen_height;
            }

            DEBUGF("stretch_blit(%d, %d, %d, %d ==> %d, %d, %d, %d)\n", src_x, src_y, src_w, src_h, dest_x, dest_y, dest_w, dest_h);
            stretch_blit(tmp, screen, src_x, src_y, src_w, src_h, dest_x, dest_y, dest_w, dest_h);

            //////
            /// draw image
            if (image_info) {
                int ySpacing = font->height + 1;
                int xPos = 20;
                int yPos = 10;
                int width = 25 * 8;
                int height = ySpacing * 9;
                if (strlen(infile) > 9) {
                    width += (strlen(infile) - 9) * 8;
                }

                rectfill(screen, xPos, yPos, xPos + width, yPos + height, makecol(32, 32, 32));
                rect(screen, xPos, yPos, xPos + width, yPos + height, makecol(227, 198, 34));

                xPos += 8;
                yPos += 8;
                int txt_col = makecol(161, 21, 158);
                textprintf_ex(screen, font, xPos, yPos, txt_col, -1, "Filename    : %s", infile);
                yPos += ySpacing;
                textprintf_ex(screen, font, xPos, yPos, txt_col, -1, "Image size  : %04dx%04d", tmp->w, tmp->h);
                yPos += ySpacing;
                textprintf_ex(screen, font, xPos, yPos, txt_col, -1, "Screen size : %04dx%04d", screen_width, screen_height);
                yPos += ySpacing;
                textprintf_ex(screen, font, xPos, yPos, txt_col, -1, "Screen bpp  : %dbpp", get_color_depth());
                yPos += ySpacing;
                textprintf_ex(screen, font, xPos, yPos, txt_col, -1, "Scaled size : %04dx%04d", scaled_width, scaled_height);
                yPos += ySpacing;
                textprintf_ex(screen, font, xPos, yPos, txt_col, -1, "Image pos   : %04dx%04d", x_start, y_start);
                yPos += ySpacing;
                textprintf_ex(screen, font, xPos, yPos, txt_col, -1, "Factor      : %.5f", factor);
                yPos += ySpacing;
            }

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
            int stepsize = 2;
            float scale_step = 1.1f;
            if (key_shifts & KB_SHIFT_FLAG) {
                stepsize *= 2;
                scale_step *= 2;
            }
            if (key_shifts & KB_CTRL_FLAG) {
                stepsize *= 4;
                scale_step *= 4;
            }
            if (key_shifts & KB_ALT_FLAG) {
                stepsize *= 8;
                scale_step *= 8;
            }

            // keys
            if ((key_upper == KEY_ESC) || (key_lower == 'Q') || (key_lower == 'q')) {
                break;  // exit
            } else if ((key_upper == KEY_LEFT) || (key_lower == '4')) {
                if (x_start > 0) {
                    x_start -= stepsize;
                }
            } else if ((key_upper == KEY_RIGHT) || (key_lower == '6')) {
                if ((scaled_width > screen_width) && (x_start + src_w < tmp->w)) {
                    x_start += stepsize;
                }
            } else if ((key_upper == KEY_UP) || (key_lower == '8')) {
                if (y_start > 0) {
                    y_start -= stepsize;
                }
            } else if ((key_upper == KEY_DOWN) || (key_lower == '2')) {
                if ((scaled_height > screen_height) && (y_start + screen_height < tmp->h)) {
                    y_start += stepsize;
                }
            } else if ((key_upper == KEY_PGDN) || (key_lower == '3')) {
                DEBUGF("factor = %f, scale_step = %f, new_factor = %f\n", factor, scale_step, factor / scale_step);
                if ((scaled_width >= screen_width / MIN_ZOOM) && (scaled_height >= screen_height / MIN_ZOOM)) {
                    factor /= scale_step;
                }
            } else if ((key_upper == KEY_PGUP) || (key_lower == '9')) {
                DEBUGF("factor = %f, scale_step = %f, new_factor = %f\n", factor, scale_step, factor * scale_step);
                factor *= scale_step;
            } else if ((key_lower == 'F') || (key_lower == 'f')) {
                // fit on screen
                if (tmp->w > tmp->h) {
                    factor = (float)screen_width / (float)tmp->w;
                } else {
                    factor = (float)screen_height / (float)tmp->h;
                }
            } else if ((key_lower == 'Z') || (key_lower == 'z')) {
                factor = 1.0f;  // full zoom
            } else if ((key_lower == 'i') || (key_lower == 'i')) {
                image_info = !image_info;
            }

            //////
            // sanitychecks
            scaled_width = tmp->w * factor;
            scaled_height = tmp->h * factor;

            if (scaled_width > screen_width) {
                src_w = (tmp->w * screen_width) / scaled_width;
                if (x_start + src_w >= tmp->w) {
                    x_start = tmp->w - src_w - 1;
                }
            }

            if (scaled_height > screen_height) {
                src_h = (tmp->h * screen_height) / scaled_height;
                if (y_start + src_h >= tmp->h) {
                    y_start = tmp->h - src_h - 1;
                }
            }

            if (x_start < 0) {
                x_start = 0;
            }
            if (y_start < 0) {
                y_start = 0;
            }
        }
        destroy_bitmap(tmp);
    } else {
        banner(stdout);
        fprintf(stdout, "Loaded %s\n", infile);
        fprintf(stdout, "Image is  %4dx%4d\n", bm->w, bm->h);
        if (scale == 1.0f) {
            if (save_bitmap(outfile, bm, NULL)) {
                destroy_bitmap(bm);
                set_last_error("Can't save image %s", outfile);
                clean_exit(EXIT_SUCCESS);
            }
            destroy_bitmap(bm);
        } else {
            scaled_width = screen_width * scale;
            scaled_height = screen_height * scale;

            if (!scaled_height || !scaled_width) {
                fprintf(stdout, "Refusing to scale down to 0 pixel.");
                clean_exit(EXIT_FAILURE);
            }

            fprintf(stdout, "Scaling to %4dx%4d\n", scaled_width, scaled_height);
            BITMAP *scaled = create_bitmap_ex(32, scaled_width, scaled_height);
            if (!scaled) {
                fprintf(stdout, "Out of memory, image to large.");
                clean_exit(EXIT_FAILURE);
            }
            stretch_blit(bm, scaled, 0, 0, bm->w, bm->h, 0, 0, scaled_width, scaled_height);
            destroy_bitmap(bm);
            if (save_bitmap(outfile, scaled, NULL)) {
                destroy_bitmap(scaled);
                set_last_error("Can't save image %s", outfile);
                clean_exit(EXIT_SUCCESS);
            }
            destroy_bitmap(scaled);
        }
        fprintf(stdout, "Wrote %s\n", outfile);
    }

    clean_exit(EXIT_SUCCESS);
}
