#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>

#include <SDL2/SDL.h>

#define TITLE "flies: genetic algorithm"
#define WIDTH 600
#define HEIGHT 600

/* configuration for drawing. */
#define DRAW_FLY_COLOR build_color(0, 0, 0)
#define DRAW_BG_COLOR build_color(190, 190, 190)
#define DRAW_OBSTACLE_COLOR build_color(90, 0, 0)
#define DRAW_GOAL_COLOR build_color(0, 200, 0)
#define DRAW_FLY_SIZE 8
#define DRAW_GOAL_SIZE GA_GOAL_SIZE

/* configuration for genetic algorithm. */
#define GA_FLY_MAX_MOVES 90
#define GA_FLY_SPEED 7
#define GA_FLY_COUNT 10
#define GA_FLY_START_X (WIDTH / 2)
#define GA_FLY_START_Y (HEIGHT * 3 / 4)
#define GA_GOAL_SIZE 8
#define GA_MUTATION_RATE 5

struct goal {
    struct goal const *next;
    int x, y;
};

struct fly {
    /*
     * using char here means that flies cannot move more than 127 units in any
     * direction; this is a compromise for lower memory usage.
     * with lower memory usage, more flies can be simulated simultaneously.
     */
    char moves_x[GA_FLY_MAX_MOVES], moves_y[GA_FLY_MAX_MOVES];
    int x, y, moves_left, moves_set, goals_reached;
    struct goal const *goal;
    bool dead;
};

static SDL_Color build_color(uint8_t r, uint8_t g, uint8_t b);
static struct fly fly_create(struct fly const *parent);
static double fly_fitness(struct fly const *fly);
static void fly_move(struct fly *fly);
static int random_int(int lower_bound, int upper_bound);
static int clamp(int lower_bound, int n, int upper_bound);

static struct fly flies[GA_FLY_COUNT];

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
        }
    }

    SDL_Quit();
    return 0;
}

static SDL_Color build_color(uint8_t r, uint8_t g, uint8_t b)
{
    SDL_Color col;

    col.r = r;
    col.g = g;
    col.b = b;
    col.a = 255;
    return col;
}

static struct fly fly_create(struct fly const *parent)
{
    struct fly fly = {0};

    if (parent != NULL) {
        memcpy(fly.moves_x, parent->moves_x, sizeof(fly.moves_x));
        memcpy(fly.moves_y, parent->moves_y, sizeof(fly.moves_y));
    }

    fly.x = GA_FLY_START_X;
    fly.y = GA_FLY_START_Y;
    fly.goal = NULL;
    fly.moves_left = GA_FLY_MAX_MOVES;
    fly.moves_set = parent == NULL ? 0 : parent->moves_set;
    fly.goals_reached = 0;
    fly.dead = false;
    return fly;
}

static double fly_fitness(struct fly const *fly)
{
}

static void fly_move(struct fly *fly)
{
    int next_move = GA_FLY_MAX_MOVES - fly->moves_left;
    
    fly->dead = fly->dead || fly->moves_left == 0;
    if (fly->dead)
        return;

    if (next_move > fly->moves_set || random_int(0, 100) <= GA_MUTATION_RATE) {
        fly->moves_x[next_move] = random_int(-GA_FLY_SPEED, GA_FLY_SPEED);
        fly->moves_y[next_move] = random_int(-GA_FLY_SPEED, GA_FLY_SPEED);
    }

    if (next_move > fly->moves_set)
        ++fly->moves_set;

    fly->x = clamp(0, fly->x + moves_x[next_move], WIDTH);
    fly->y = clamp(0, fly->y + moves_y[next_move], HEIGHT);
    --fly->moves_left;
}

static int random_int(int lower_bound, int upper_bound)
{
    if (lower_bound > upper_bound) {
        printf("tried to generate random integer where LB > UB\n");
        exit(-1);
    }

    return rand() % (upper_bound - lower_bound + 1) + lower_bound;
}

static int clamp(int lower_bound, int n, int upper_bound)
{
    if (lower_bound > upper_bound) {
        printf("tried to clamp a value where LB > UB\n");
        exit(-1);
    }

    return n < upper_bound ? n > lower_bound ? n : lower_bound : upper_bound;
}
