#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "autil.h"
#include "aengn.h"

#ifdef __EMSCRIPTEN__
#    include <emscripten.h>
#endif

#define VERSION "0.1"

static struct rgba const black = {0x00, 0x00, 0x00, 0xff};
static struct rgba const white = {0xff, 0xff, 0xff, 0xff};
static struct rgba const iconc = {0xaa, 0xaa, 0xaa, 0x77};
static struct rgba const red = {0xff, 0x00, 0x00, 0xff};

#define CELLS_W 64
#define CELLS_H 48
// OFFSET accounts for the always-dead 1-cell boundary around the visible
// region of cells on screen. Keeping this extra boundary allows for the
// calculation of cell neighbors to use (x|y)-1 and (x|y)+1 without ever
// indexing out of bounds.
#define OFFSET 1
// cells[x][y] == 1 => Cell is alive.
// cells[x][y] == 0 => Cell is dead.
static int cells[CELLS_W + 2 * OFFSET][CELLS_H + 2 * OFFSET];
// neighbors[x][y] => Number of neighbors that the cell at (x, y) has.
static int neighbors[CELLS_W + 2 * OFFSET][CELLS_H + 2 * OFFSET];

// Advance one generation per frame if true.
static bool run = false;
// Advance one generation and then pause if true.
static bool step = false;
// Hide cursor and UI icons (play and pause) if true.
static bool hide = false;

static struct sprite* spr_cells = NULL;
static struct sprite* spr_mouse = NULL;
static struct sprite* spr_icon_play = NULL;
static struct sprite* spr_icon_pause = NULL;

static void
usage(void);
static void
argparse(int argc, char** argv);

static int
main_init(void);
static void
main_fini(void);
static void
main_loop_body(void);

int
onevent(SDL_Event* event, void* ctx);
static void
update(void);
static void
render(void);

int
main(int argc, char** argv)
{
    argparse(argc, argv);

    static unsigned const PIXEL_SCALE = 16;
    if (aengn_init(CELLS_W, CELLS_H, PIXEL_SCALE) != 0) {
        return EXIT_FAILURE;
    }
    atexit(aengn_fini);

    if (main_init() != 0) {
        return EXIT_FAILURE;
    }
    atexit(main_fini);

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(main_loop_body, 0, 1);
#else
    while (!should_quit()) {
        main_loop_body();
    }
#endif

    return EXIT_SUCCESS;
}

static void
usage(void)
{
    // clang-format off
    fputs(
        "Usage: life [OPTION]... FILE"                                  "\n"
        "Options:"                                                      "\n"
        "  -h, --help       Display usage information and exit."        "\n"
        "      --version    Display version information and exit."      "\n"
        "Controls:"                                                     "\n"
        "   Left mouse button   => Turn cell under cursor alive."       "\n"
        "   Right mouse button  => Turn cell under cursor dead."        "\n"
        "   Middle mouse button => Turn cell under cursor dead"         "\n"
        "                          (same as right mouse button)."       "\n"
        "   R or RETURN         => Start / stop simulation."            "\n"
        "   S or SPACE          => Advance one generation."             "\n"
        "   C                   => Turn all cells on the board dead."   "\n"
        "   H                   => Hide / show cursor & UI icons."      "\n"
        , stderr
    );
    // clang-format on
}

static void
argparse(int argc, char** argv)
{
    bool error_unrecognized_option = false;
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            usage();
            exit(EXIT_SUCCESS);
        }
        if (strcmp(argv[i], "--version") == 0) {
            puts(VERSION);
            exit(EXIT_SUCCESS);
        }

        if (strncmp(argv[i], "-", 1) == 0 || strncmp(argv[i], "--", 2) == 0) {
            errorf("Unrecognized command line option '%s'", argv[i]);
            error_unrecognized_option = true;
            continue;
        }
    }

    if (error_unrecognized_option) {
        exit(EXIT_FAILURE);
    }
}

static int
main_init(void)
{
    spr_cells = sprite_new(screen_w() + OFFSET, screen_h() + OFFSET);
    if (spr_cells == NULL) {
        goto error;
    }

    spr_mouse = sprite_new(1, 1);
    if (spr_mouse == NULL) {
        goto error;
    }
    sprite_set_pixel(spr_mouse, 0, 0, &red);

    static int const ICON_W = 3;
    static int const ICON_H = 3;

    spr_icon_play = sprite_new(ICON_W, ICON_H);
    if (spr_icon_play == NULL) {
        goto error;
    }
    sprite_set_pixel(spr_icon_play, 0, 0, &iconc);
    sprite_set_pixel(spr_icon_play, 0, 1, &iconc);
    sprite_set_pixel(spr_icon_play, 0, 2, &iconc);
    sprite_set_pixel(spr_icon_play, 1, 0, &iconc);
    sprite_set_pixel(spr_icon_play, 1, 1, &iconc);
    sprite_set_pixel(spr_icon_play, 1, 2, &iconc);
    sprite_set_pixel(spr_icon_play, 2, 1, &iconc);

    spr_icon_pause = sprite_new(ICON_W, ICON_H);
    if (spr_icon_pause == NULL) {
        goto error;
    }
    sprite_set_pixel(spr_icon_pause, 0, 0, &iconc);
    sprite_set_pixel(spr_icon_pause, 0, 1, &iconc);
    sprite_set_pixel(spr_icon_pause, 0, 2, &iconc);
    sprite_set_pixel(spr_icon_pause, 2, 0, &iconc);
    sprite_set_pixel(spr_icon_pause, 2, 1, &iconc);
    sprite_set_pixel(spr_icon_pause, 2, 2, &iconc);

    return 0;

error:
    main_fini();
    return -1;
}

static void
main_fini(void)
{
    if (spr_cells != NULL) {
        sprite_del(spr_cells);
    }
    if (spr_mouse != NULL) {
        sprite_del(spr_mouse);
    }
    if (spr_icon_play != NULL) {
        sprite_del(spr_icon_play);
    }
    if (spr_icon_pause != NULL) {
        sprite_del(spr_icon_pause);
    }
}

static void
main_loop_body(void)
{
    aengn_frame_begin(onevent, NULL);
    update();
    render();
    aengn_frame_end();
}

int
onevent(SDL_Event* event, void* ctx)
{
    (void)ctx;

    int quit = 0;
    if (event->type == SDL_QUIT) {
        quit = true;
    }
    if (event->type == SDL_KEYUP && event->key.keysym.sym == SDLK_ESCAPE) {
        quit = true;
    }

    return quit;
}

static void
update(void)
{
    // Left mouse button => Turn cell under mouse alive.
    if (mousebutton_state(MOUSEBUTTON_LEFT)->down) {
        cells[mousepos_x() + OFFSET][mousepos_y() + OFFSET] = 1;
    }
    // Middle mouse button => Turn cell under mouse dead.
    // Right mouse button => Turn cell under mouse dead.
    else if (
        mousebutton_state(MOUSEBUTTON_MIDDLE)->down
        || mousebutton_state(MOUSEBUTTON_RIGHT)->down) {
        cells[mousepos_x() + OFFSET][mousepos_y() + OFFSET] = 0;
    }

    // R or RETURN => Start / stop simulation.
    if (scankey_state(SDL_SCANCODE_RETURN)->pressed
        || virtkey_state(SDLK_r)->pressed) {
        run = !run;
        step = false;
        infof("run => %s", run ? "true" : "false");
    }
    // S or SPACE => Advance one generation.
    if (scankey_state(SDL_SCANCODE_SPACE)->pressed
        || virtkey_state(SDLK_s)->pressed) {
        infof("step");
        run = false;
        step = true;
    }
    // C => Turn all cells on the board dead.
    if (virtkey_state(SDLK_c)->pressed) {
        run = false;
        memset(cells, 0x00, sizeof(cells));
    }
    // H => Hide / show cursor & UI icons.
    if (virtkey_state(SDLK_h)->pressed) {
        hide = !hide;
    }

    // Update the  cell-grid sprite to match the cells array.
    // The grid will get drawn regardless of whether the simulation is paused
    // or not, so it should be updated every frame.
    for (int x = OFFSET; x < CELLS_W + OFFSET; ++x) {
        for (int y = OFFSET; y < CELLS_H + OFFSET; ++y) {
            struct rgba const* color = cells[x][y] != 0 ? &white : &black;
            sprite_set_pixel(spr_cells, x, y, color);
        }
    }

    bool const paused = !run && !step;
    if (paused) {
        return;
    }
    step = 0;

    memset(neighbors, 0x00, sizeof(neighbors));
    for (int x = OFFSET; x < CELLS_W + OFFSET; ++x) {
        for (int y = OFFSET; y < CELLS_H + OFFSET; ++y) {
            if (cells[x][y] == 0) {
                continue;
            }
            neighbors[x - 1][y - 1] += 1;
            neighbors[x + 0][y - 1] += 1;
            neighbors[x + 1][y - 1] += 1;

            neighbors[x - 1][y + 0] += 1;
            neighbors[x + 1][y + 0] += 1;

            neighbors[x - 1][y + 1] += 1;
            neighbors[x + 0][y + 1] += 1;
            neighbors[x + 1][y + 1] += 1;
        }
    }
    for (int x = OFFSET; x < CELLS_W + OFFSET; ++x) {
        for (int y = OFFSET; y < CELLS_H + OFFSET; ++y) {
            cells[x][y] = cells[x][y] != 0
                ? neighbors[x][y] == 2 || neighbors[x][y] == 3
                : neighbors[x][y] == 3;
        }
    }
}

static void
render(void)
{
    draw_clear(RGBA_PTR(0x00, 0x00, 0x00, 0xff));
    draw_sprite(spr_cells, -OFFSET, -OFFSET);
    if (!hide) {
        draw_sprite(run || step ? spr_icon_play : spr_icon_pause, 1, 1);
        draw_sprite(spr_mouse, mousepos_x(), mousepos_y());
    }
    draw_show();
}

#define AUTIL_IMPLEMENTATION
#include "autil.h"
#define AENGN_IMPLEMENTATION
#include "aengn.h"
