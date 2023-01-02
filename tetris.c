#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

#include <SDL2/SDL.h>

/* game settings. */
#define TPS 60
#define FIELD_WIDTH 10
#define FIELD_HEIGHT 20
#define TETRO_WIDTH 4
#define TETRO_HEIGHT 4

/* rendering settings. */
#define TITLE "tetris"
#define BLOCK_SIZE 30
#define WIDTH (FIELD_WIDTH * BLOCK_SIZE)
#define HEIGHT (FIELD_HEIGHT * BLOCK_SIZE)

static SDL_Keycode remap_keycode(SDL_Keycode kc);
static void update_key_states(SDL_Event const *e);
static bool key_pressed(SDL_Keycode kc);
static bool valid_tetro_state(uint8_t const **tetro, int row, int col);
static void rotate_tetro(uint8_t **tetro);

static bool key_states[0x21b];
static uint8_t field[FIELD_HEIGHT][FIELD_WIDTH];
static uint8_t cur_tetro[TETRO_HEIGHT][TETRO_WIDTH];
static int cur_row = 0, cur_col = 0;

static const uint8_t piece_t[4][4] = {
    {0, 0, 0, 0},
    {0, 1, 1, 1},
    {0, 0, 1, 0},
    {0, 0, 0, 0},
};

static const uint8_t piece_j[4][4] = {
    {0, 0, 0, 0},
    {0, 0, 1, 0},
    {0, 0, 1, 0},
    {0, 1, 1, 0},
};

static const uint8_t piece_z[4][4] = {
    {0, 0, 0, 0},
    {0, 1, 1, 0},
    {0, 0, 1, 1},
    {0, 0, 0, 0},
};

static const uint8_t piece_o[4][4] = {
    {0, 0, 0, 0},
    {0, 1, 1, 0},
    {0, 1, 1, 0},
    {0, 0, 0, 0},
};

static const uint8_t piece_s[4][4] = {
    {0, 0, 0, 0},
    {0, 0, 1, 1},
    {0, 1, 1, 0},
    {0, 0, 0, 0},
};

static const uint8_t piece_l[4][4] = {
    {0, 0, 0, 0},
    {0, 1, 0, 0},
    {0, 1, 0, 0},
    {0, 1, 1, 0},
};

static const uint8_t piece_i[4][4] = {
    {0, 0, 1, 0},
    {0, 0, 1, 0},
    {0, 0, 1, 0},
    {0, 0, 1, 0},
};

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
        
        while (SDL_PollEvent(&e) == 1) {
            if (e.type == SDL_QUIT)
                running = false;

            update_key_states(&e);
        }

        SDL_Delay(1000 / TPS);
    }
    
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
