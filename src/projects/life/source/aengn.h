/*
AENGN - ASHN'S GAME ENGINE
    Single header file containing the guts of a 2D engine for games and simple
    applications with pixel graphics.
    IMPORTANT NOTE: This is a work-in-progress library created for the OLC
    Codejam 2020. It is iscomplete and not very well documented. Use at your
    own risk.

LICENSE
    Copyright (c) 2020 ashn <me@ashn.dev>

    Permission to use, copy, modify, and/or distribute this software for any
    purpose with or without fee is hereby granted.

    THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
    WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
    MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
    SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
    WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
    OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
    CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/

////////////////////////////////////////////////////////////////////////////////
//////////////////////////////// HEADER SECTION ////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#ifndef AENGN_H_INCLUDED
#define AENGN_H_INCLUDED

#ifndef AENGN_API
#    define AENGN_API extern
#endif

#ifndef AUTIL_H_INCLUDED
#    error "autil.h must be included before this file"
#endif

#include <stdbool.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

// clang-format off
#define MOUSEBUTTON_LEFT   ((int)0)
#define MOUSEBUTTON_RIGHT  ((int)1)
#define MOUSEBUTTON_MIDDLE ((int)2)
#define MOUSEBUTTON_COUNT  ((size_t)3)
// clang-format on

AENGN_API int
aengn_init(unsigned screen_w, unsigned screen_h, unsigned pixel_scale);
AENGN_API void
aengn_fini(void);

// Returns non-zero if should quit.
typedef int (*onevent_fn)(SDL_Event* event, void* ctx);

AENGN_API void
aengn_frame_begin(onevent_fn onevent, void* ctx);
AENGN_API void
aengn_frame_end(void);

AENGN_API int
screen_w(void);
AENGN_API int
screen_h(void);
AENGN_API int
pixel_scale(void);
AENGN_API bool
should_quit(void);

struct button_state
{
    bool pressed;
    bool released;
    bool down;
};
AENGN_API struct button_state const*
scankey_state(SDL_Scancode key);
AENGN_API struct button_state const*
virtkey_state(SDL_Keycode key);
AENGN_API struct button_state const*
mousebutton_state(int button);

// Returns position in the range [0, screen_w()-1].
AENGN_API int
mousepos_x(void);
// Returns position in the range [0, screen_h()-1].
AENGN_API int
mousepos_y(void);

struct sprite;
struct rgba
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
};
#define RGBA_PTR(r_, g_, b_, a_)                                               \
    (&(struct rgba){.r = r_, .g = g_, .b = b_, .a = a_})
AENGN_API struct sprite*
sprite_new(int w, int h);
AENGN_API void
sprite_del(struct sprite* self);
AENGN_API int
sprite_w(struct sprite const* self);
AENGN_API int
sprite_h(struct sprite const* self);
AENGN_API void
sprite_set_pixel(struct sprite* self, int x, int y, struct rgba const* color);
AENGN_API void
sprite_get_pixel(struct sprite const* self, int x, int y, struct rgba* color);
// Automatically called by draw_sprite if needed.
AENGN_API int
sprite_update_texture(struct sprite* self);

AENGN_API SDL_Surface*
load_surface(char const* path);
AENGN_API SDL_Texture*
load_texture(char const* path);
AENGN_API struct sprite*
load_sprite(char const* path);

AENGN_API void
draw_clear(struct rgba const* color);
AENGN_API int
draw_texture(SDL_Texture* tex, int x, int y);
AENGN_API int
draw_sprite(struct sprite* sprite, int x, int y);
AENGN_API void
draw_show(void);

#endif // AENGN_H_INCLUDED

////////////////////////////////////////////////////////////////////////////////
//////////////////////////// IMPLEMENTATION SECTION ////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#ifdef AENGN_IMPLEMENTATION
#undef AENGN_IMPLEMENTATION

static int g_screen_w = 0;
static int g_screen_h = 0;
static int g_pixel_scale = 0;
static bool g_should_quit = false;

static struct button_state BUTTON_STATE_DEFAULT = {0};
// SDL_Scancode => struct button_state
static struct map* g_scankey_map = NULL;
static int
scankey_map_vpcmp(void const* lhs, void const* rhs)
{
    assert(lhs != NULL);
    assert(rhs != NULL);
    SDL_Scancode const l = *(SDL_Scancode*)lhs;
    SDL_Scancode const r = *(SDL_Scancode*)rhs;
    if (l < r) {
        return -1;
    }
    if (l > r) {
        return +1;
    }
    return 0;
}
// SDL_Keycode => struct button_state
static struct map* g_virtkey_map = NULL;
static int
virtkey_map_vpcmp(void const* lhs, void const* rhs)
{
    assert(lhs != NULL);
    assert(rhs != NULL);
    SDL_Keycode const l = *(SDL_Keycode*)lhs;
    SDL_Keycode const r = *(SDL_Keycode*)rhs;
    if (l < r) {
        return -1;
    }
    if (l > r) {
        return +1;
    }
    return 0;
}
// Array mapping AENGN-supported mouse buttons => struct button state.
static struct button_state g_mousebutton_state[MOUSEBUTTON_COUNT];

static SDL_Window* g_window = NULL;
static SDL_Renderer* g_renderer = NULL;

AENGN_API int
aengn_init(unsigned screen_w, unsigned screen_h, unsigned pixel_scale)
{
    g_screen_w = (int)screen_w;
    g_screen_h = (int)screen_h;
    g_pixel_scale = (int)pixel_scale;
    g_should_quit = false;
    g_scankey_map = map_new(
        sizeof(SDL_Scancode), sizeof(struct button_state), scankey_map_vpcmp);
    g_virtkey_map = map_new(
        sizeof(SDL_Keycode), sizeof(struct button_state), virtkey_map_vpcmp);
    memset(g_mousebutton_state, 0x00, sizeof(g_mousebutton_state));

    SDL_version sdlver;
    SDL_GetVersion(&sdlver);
    infof(
        "Using SDL version %d.%d.%d", sdlver.major, sdlver.minor, sdlver.patch);

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        errorf("[%s][SDL_Init] %s", __func__, SDL_GetError());
        goto error;
    }

    g_window = SDL_CreateWindow(
        "",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        g_screen_w * g_pixel_scale,
        g_screen_h * g_pixel_scale,
        SDL_WINDOW_SHOWN);
    if (g_window == NULL) {
        errorf("[%s][SDL_CreateWindow] %s", __func__, SDL_GetError());
        goto error;
    }

    g_renderer = SDL_CreateRenderer(
        g_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (g_renderer == NULL) {
        errorf("[%s][SDL_CreateRenderer] %s", __func__, SDL_GetError());
        goto error;
    }
    SDL_SetRenderDrawBlendMode(g_renderer, SDL_BLENDMODE_BLEND);

    return 0;

error:
    aengn_fini();
    return -1;
}

AENGN_API void
aengn_fini(void)
{
    if (g_window != NULL) {
        SDL_DestroyWindow(g_window);
        g_window = NULL;
    }
    if (g_renderer != NULL) {
        SDL_DestroyRenderer(g_renderer);
        g_renderer = NULL;
    }

    if (SDL_WasInit(SDL_INIT_EVERYTHING)) {
        SDL_Quit();
    }

    g_screen_w = -1;
    g_screen_h = -1;
    g_pixel_scale = -1;
    g_should_quit = false;

    assert(g_scankey_map != NULL);
    map_del(g_scankey_map);
    g_scankey_map = NULL;

    assert(g_virtkey_map != NULL);
    map_del(g_virtkey_map);
    g_virtkey_map = NULL;

    memset(g_mousebutton_state, 0x00, sizeof(g_mousebutton_state)); // scrub
}

static int
aengn_poll_event(SDL_Event* event)
{
    if (!SDL_PollEvent(event)) {
        return 0;
    }

    // SDL_KeyboardEvent
    SDL_Scancode scankey = (SDL_Scancode)0xDEADBEEF;
    SDL_Keycode virtkey = (SDL_Keycode)0xDEADBEEF;
    struct button_state* scanstate = NULL;
    struct button_state* virtstate = NULL;
    if (event->type == SDL_KEYDOWN || event->type == SDL_KEYUP) {
        scankey = event->key.keysym.scancode;
        virtkey = event->key.keysym.sym;
        scanstate = map_lookup(g_scankey_map, &scankey);
        virtstate = map_lookup(g_virtkey_map, &virtkey);
        if (scanstate == NULL) {
            map_insert(
                g_scankey_map, &scankey, &BUTTON_STATE_DEFAULT, NULL, NULL);
            scanstate = map_lookup(g_scankey_map, &scankey);
        }
        if (virtstate == NULL) {
            map_insert(
                g_virtkey_map, &virtkey, &BUTTON_STATE_DEFAULT, NULL, NULL);
            virtstate = map_lookup(g_virtkey_map, &virtkey);
        }
        assert(scanstate != NULL);
        assert(virtstate != NULL);
    }
    if (event->type == SDL_KEYDOWN) {
        assert(scanstate != NULL);
        assert(virtstate != NULL);
        scanstate->pressed = true;
        virtstate->pressed = true;
        scanstate->down = true;
        virtstate->down = true;
    }
    if (event->type == SDL_KEYUP) {
        assert(scanstate != NULL);
        assert(virtstate != NULL);
        scanstate->released = true;
        virtstate->released = true;
        scanstate->down = false;
        virtstate->down = false;
    }

    // SDL_MouseButtonEvent
    struct button_state* mousebtnstate = NULL;
    if (event->type == SDL_MOUSEBUTTONDOWN
        || event->type == SDL_MOUSEBUTTONUP) {
        if (event->button.button == SDL_BUTTON_LEFT) {
            mousebtnstate = &g_mousebutton_state[MOUSEBUTTON_LEFT];
        }
        if (event->button.button == SDL_BUTTON_RIGHT) {
            mousebtnstate = &g_mousebutton_state[MOUSEBUTTON_RIGHT];
        }
        if (event->button.button == SDL_BUTTON_MIDDLE) {
            mousebtnstate = &g_mousebutton_state[MOUSEBUTTON_MIDDLE];
        }
    }
    if (event->type == SDL_MOUSEBUTTONDOWN && mousebtnstate != NULL) {
        mousebtnstate->pressed = true;
        mousebtnstate->down = true;
    }
    if (event->type == SDL_MOUSEBUTTONUP && mousebtnstate != NULL) {
        mousebtnstate->released = true;
        mousebtnstate->down = false;
    }

    return 1;
}

AENGN_API void
aengn_frame_begin(onevent_fn onevent, void* ctx)
{
    struct vec const* const scankeys = map_vals(g_scankey_map);
    for (size_t i = 0; i < vec_count(scankeys); ++i) {
        struct button_state* const sk = vec_get(scankeys, i);
        assert(sk != NULL);
        sk->pressed = false;
        sk->released = false;
    }
    struct vec const* const virtkeys = map_vals(g_virtkey_map);
    for (size_t i = 0; i < vec_count(virtkeys); ++i) {
        struct button_state* const vk = vec_get(virtkeys, i);
        assert(vk != NULL);
        vk->pressed = false;
        vk->released = false;
    }
    for (size_t i = 0; i < MOUSEBUTTON_COUNT; ++i) {
        struct button_state* const btn = &g_mousebutton_state[i];
        btn->pressed = false;
        btn->released = false;
    }

    // Grab all events.
    SDL_Event e;
    while (aengn_poll_event(&e)) {
        if (onevent == NULL) {
            continue;
        }
        if (onevent(&e, ctx) != 0) {
            g_should_quit = true;
        }
    }
}

AENGN_API void
aengn_frame_end(void)
{
    static Uint32 fps_period_start = 0;
    static size_t fps_period_frames = 0;
    static Uint32 const SDL_TICKS_PER_SEC = 1000; // Millisec

    fps_period_frames += 1;
    Uint32 const now = SDL_GetTicks();
    if ((now - fps_period_start) > SDL_TICKS_PER_SEC) {
        char title[256] = {0};
        snprintf(title, ARRAY_COUNT(title) - 1, "FPS: %zu", fps_period_frames);
        SDL_SetWindowTitle(g_window, title);

        fps_period_start = now;
        fps_period_frames = 0;
    }
}

AENGN_API int
screen_w(void)
{
    return g_screen_w;
}

AENGN_API int
screen_h(void)
{
    return g_screen_h;
}

AENGN_API int
pixel_scale(void)
{
    return g_pixel_scale;
}

AENGN_API bool
should_quit(void)
{
    return g_should_quit;
}

AENGN_API struct button_state const*
scankey_state(SDL_Scancode key)
{
    struct button_state const* const state = map_lookup(g_scankey_map, &key);
    return state != NULL ? state : &BUTTON_STATE_DEFAULT;
}

AENGN_API struct button_state const*
virtkey_state(SDL_Keycode key)
{
    struct button_state const* const state = map_lookup(g_virtkey_map, &key);
    return state != NULL ? state : &BUTTON_STATE_DEFAULT;
}

AENGN_API struct button_state const*
mousebutton_state(int button)
{
    assert(0 <= button && button < (int)MOUSEBUTTON_COUNT);
    return &g_mousebutton_state[button];
}

AENGN_API int
mousepos_x(void)
{
    int x;
    SDL_GetMouseState(&x, NULL);
    return x / g_pixel_scale;
}

AENGN_API int
mousepos_y(void)
{
    int y;
    SDL_GetMouseState(NULL, &y);
    return y / g_pixel_scale;
}

struct sprite
{
    int w;
    int h;
    SDL_Surface* surface;
    SDL_Texture* texture;
    bool texture_needs_update;
};

AENGN_API struct sprite*
sprite_new(int w, int h)
{
    assert(w > 0);
    assert(h > 0);

    struct sprite* self = NULL;
    SDL_Surface* surface = NULL;
    SDL_Texture* texture = NULL;
    static const int depth = 32;

    self = xalloc(NULL, sizeof(struct sprite));

    surface = SDL_CreateRGBSurfaceWithFormat(
        0, w, h, depth, SDL_PIXELFORMAT_ARGB8888);
    if (surface == NULL) {
        errorf(
            "[%s][SDL_CreateRGBSurfaceWithFormat] %s",
            __func__,
            SDL_GetError());
        goto error;
    }

    texture = SDL_CreateTexture(
        g_renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        w,
        h);
    if (texture == NULL) {
        errorf("[%s][SDL_CreateTexture] %s", __func__, SDL_GetError());
        goto error;
    }
    // TODO: Using alpha blending on all sprites *may* lead to a performance
    //       hit. Do testing with SDL_BLENDMODE_NONE to see if there is a
    //       performance difference, and if so, maybe make the blend mode
    //       configurable.
    if (0 != SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND)) {
        errorf("[%s][SDL_SetTextureBlendMode] %s", __func__, SDL_GetError());
        goto error;
    }

    self->w = w;
    self->h = h;
    self->surface = surface;
    self->texture = texture;
    self->texture_needs_update = false;

    return self;

error:
    if (self != NULL) {
        xalloc(self, XALLOC_FREE);
    }
    if (surface != NULL) {
        SDL_FreeSurface(surface);
    }
    if (texture != NULL) {
        SDL_DestroyTexture(texture);
    }
    return NULL;
}

AENGN_API void
sprite_del(struct sprite* self)
{
    assert(self != NULL);

    SDL_FreeSurface(self->surface);
    SDL_DestroyTexture(self->texture);
    memset(self, 0x00, sizeof(*self)); // scrub
    xalloc(self, XALLOC_FREE);
}

AENGN_API int
sprite_w(struct sprite const* self)
{
    assert(self != NULL);

    return self->w;
}

AENGN_API int
sprite_h(struct sprite const* self)
{
    assert(self != NULL);

    return self->h;
}

AENGN_API int
sprite_update_texture(struct sprite* self)
{
    int const err = SDL_UpdateTexture(
        self->texture, NULL, self->surface->pixels, self->surface->pitch);
    if (err) {
        errorf("[%s][SDL_UpdateTexture] %s", __func__, SDL_GetError());
    }

    self->texture_needs_update = false;
    return err;
}

static Uint32*
sprite_locate_pixel(struct sprite const* sprite, int x, int y)
{
    assert(0 <= x && x < sprite->w);
    assert(0 <= y && y < sprite->h);

    Uint32* const pixels = sprite->surface->pixels;
    int const rowlen =
        sprite->surface->pitch / sprite->surface->format->BytesPerPixel;
    return pixels + (y * rowlen) + x;
}

AENGN_API void
sprite_set_pixel(struct sprite* self, int x, int y, struct rgba const* color)
{
    assert(self != NULL);
    assert(color != NULL);

    Uint32* const px = sprite_locate_pixel(self, x, y);
    Uint32 const rgba = SDL_MapRGBA(
        self->surface->format, color->r, color->g, color->b, color->a);
    *px = rgba;
    self->texture_needs_update = true;
}

AENGN_API void
sprite_get_pixel(struct sprite const* self, int x, int y, struct rgba* color)
{
    assert(self != NULL);
    assert(color != NULL);

    Uint32* const px = sprite_locate_pixel(self, x, y);
    SDL_GetRGBA(
        *px, self->surface->format, &color->r, &color->g, &color->b, &color->a);
}

AENGN_API SDL_Surface*
load_surface(char const* path)
{
    SDL_Surface* const surface = IMG_Load(path);
    if (surface == NULL) {
        errorf("[%s][IMG_Load] %s", __func__, IMG_GetError());
        return NULL;
    }
    return surface;
}

AENGN_API SDL_Texture*
load_texture(char const* path)
{
    SDL_Surface* const surface = load_surface(path);
    if (surface == NULL) {
        errorf("[%s][load_surface] Failed to load surface %s", __func__, path);
        return NULL;
    }

    SDL_Texture* const texture =
        SDL_CreateTextureFromSurface(g_renderer, surface);
    SDL_FreeSurface(surface);
    if (texture == NULL) {
        errorf(
            "[%s(%s)][SDL_CreateTextureFromSurface][%s] %s",
            __func__,
            path,
            SDL_GetError());
        return NULL;
    }

    return texture;
}

AENGN_API struct sprite*
load_sprite(char const* path)
{
    int err = 0;
    SDL_Surface* surface = NULL;
    struct sprite* sprite = NULL;

    surface = load_surface(path);
    if (surface == NULL) {
        errorf("[%s][load_surface] Failed to load surface %s", __func__, path);
        goto error;
    }

    sprite = sprite_new(surface->w, surface->h);
    if (sprite == NULL) {
        errorf("[%s][sprite_new] Failed create sprite", __func__);
        goto error;
    }

    err = SDL_BlitSurface(surface, NULL, sprite->surface, NULL);
    if (err) {
        errorf("[%s][SDL_BlitSurface] %s", __func__, SDL_GetError());
        goto error;
    }

    err = sprite_update_texture(sprite);
    if (err) {
        errorf(
            "[%s][sprite_update_texture] Failed to update texture after loading",
            __func__);
        goto error;
    }

    return sprite;

error:
    if (surface != NULL) {
        SDL_FreeSurface(surface);
    }
    if (sprite != NULL) {
        sprite_del(sprite);
    }
    return NULL;
}

AENGN_API int
draw_texture(SDL_Texture* tex, int x, int y)
{
    int w;
    int h;
    SDL_QueryTexture(tex, NULL, NULL, &w, &h);
    SDL_Rect dst = {.x = x * g_pixel_scale,
                    .y = y * g_pixel_scale,
                    .w = w * g_pixel_scale,
                    .h = h * g_pixel_scale};
    int const err = SDL_RenderCopy(g_renderer, tex, NULL, &dst);
    if (err) {
        errorf("[%s][SDL_RenderCopy] %s", __func__, SDL_GetError());
    }
    return err;
}

AENGN_API int
draw_sprite(struct sprite* sprite, int x, int y)
{
    assert(sprite != NULL);

    if (sprite->texture_needs_update) {
        sprite_update_texture(sprite);
    }

    int const err = draw_texture(sprite->texture, x, y);
    if (err) {
        errorf("[%s][draw_texture] Failed to draw sprite texture", __func__);
    }
    return err;
}

AENGN_API void
draw_clear(struct rgba const* color)
{
    static struct rgba const black = {0x00, 0x00, 0x00, SDL_ALPHA_OPAQUE};
    if (color == NULL) {
        color = &black;
    }
    SDL_SetRenderDrawColor(g_renderer, color->r, color->g, color->b, color->a);
    SDL_RenderClear(g_renderer);
}

AENGN_API void
draw_show(void)
{
    SDL_RenderPresent(g_renderer);
}

#endif // AENGN_IMPLEMENTATION
