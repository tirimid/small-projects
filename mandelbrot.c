#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdint.h>

#include <SDL2/SDL.h>

#define TITLE "mandelbrot fractal visualization"
#define WIDTH 600
#define HEIGHT 600

/* colemak keyboard layout used here. */
#define KEY_MOVE_UP SDLK_w
#define KEY_MOVE_LEFT SDLK_a
#define KEY_MOVE_DOWN SDLK_r
#define KEY_MOVE_RIGHT SDLK_s
#define KEY_ZOOM_IN SDLK_p
#define KEY_ZOOM_OUT SDLK_t
#define KEY_INCREASE_DEPTH SDLK_g
#define KEY_DECREASE_DEPTH SDLK_d

static SDL_Keycode remap_keycode(SDL_Keycode kc);
static void update_key_states(SDL_Event const *e);
static bool key_pressed(SDL_Keycode kc);
static void get_move_dirs(int *out_x, int *out_y);
static int escape_time(double x0, double y0);

static bool key_states[0x21b];
static int depth = 100;
static double offset_x = -2.2, offset_y = -1.6, size_x = 3.0, size_y = 3.0;
static double speed = 0.07, zoom_speed = 1.1;

int main(void)
{
    SDL_Window *wnd;
    SDL_Renderer *rend;
    bool running = true;
    
    SDL_Init(SDL_INIT_VIDEO);

    if ((wnd = SDL_CreateWindow(TITLE, 0, 0, WIDTH, HEIGHT, 0)) == NULL) {
        printf("failed to create window\n");
        return -1;
    }

    if ((rend = SDL_CreateRenderer(wnd, -1, 0)) == NULL) {
        printf("failed to create renderer\n");
        return -1;
    }

    while (running) {
        SDL_Event e;
        int i, j, move_dir_x = 0, move_dir_y = 0;
        
        while (SDL_PollEvent(&e) == 1) {
            if (e.type == SDL_QUIT)
                running = false;

            update_key_states(&e);
        }

        get_move_dirs(&move_dir_x, &move_dir_y);
        offset_x += speed * move_dir_x * size_x;
        offset_y += speed * move_dir_y * size_y;

        if (key_pressed(KEY_ZOOM_IN)) {
            offset_x += (size_x - size_x / zoom_speed) / 2;
            offset_y += (size_y - size_y / zoom_speed) / 2;
            size_x /= zoom_speed;
            size_y /= zoom_speed;
        }

        if (key_pressed(KEY_ZOOM_OUT)) {
            offset_x -= (size_x * zoom_speed - size_x) / 2;
            offset_y -= (size_y * zoom_speed - size_y) / 2;
            size_x *= zoom_speed;
            size_y *= zoom_speed;
        }

        if (key_pressed(KEY_INCREASE_DEPTH))
            ++depth;

        if (key_pressed(KEY_DECREASE_DEPTH))
            depth -= depth <= 1 ? 0 : 1;
        
        for (i = 0; i < WIDTH; ++i) {
            for (j = 0; j < HEIGHT; ++j) {
                double x0, y0;
                uint8_t col;
                
                x0 = (double)i / (double)WIDTH * size_x + offset_x;
                y0 = (double)j / (double)HEIGHT * size_y + offset_y;
                col = (double)escape_time(x0, y0) / (double)depth * 255.0;
                SDL_SetRenderDrawColor(rend, 0, col, 0, 255);
                SDL_RenderDrawPoint(rend, i, j);
            }
        }
        
        SDL_RenderPresent(rend);
    }

    SDL_DestroyRenderer(rend);
    SDL_DestroyWindow(wnd);
    SDL_Quit();
    return 0;
}

static SDL_Keycode remap_keycode(SDL_Keycode kc)
{
    return (kc & 1 << 30) > 0 ? (kc & ~(1 << 30)) + 0x100 : kc;
}

static void update_key_states(SDL_Event const *e)
{
    SDL_Keycode kc;

    kc = SDL_GetKeyFromScancode(e->key.keysym.scancode);
    kc = remap_keycode(kc);
    
    switch (e->type) {
    case SDL_KEYDOWN:
        key_states[kc] = true;
        break;
    case SDL_KEYUP:
        key_states[kc] = false;
        break;
    }
}

static bool key_pressed(SDL_Keycode kc)
{
    return key_states[remap_keycode(kc)];
}

static void get_move_dirs(int *out_x, int *out_y)
{
    if (key_pressed(KEY_MOVE_UP))
        --*out_y;
    
    if (key_pressed(KEY_MOVE_LEFT))
        --*out_x;
    
    if (key_pressed(KEY_MOVE_DOWN))
        ++*out_y;

    if (key_pressed(KEY_MOVE_RIGHT))
        ++*out_x;
}

static int escape_time(double x0, double y0)
{
    double x = 0, y = 0, x2 = 0, y2 = 0, w = 0;
    int i;

    for (i = 0; x2 + y2 <= 4 && i < depth; ++i) {
        x = x2 - y2 + x0;
        y = w - x2 - y2 + y0;
        x2 = x * x;
        y2 = y * y;
        w = (x + y) * (x + y);
    }

    return i;
}
