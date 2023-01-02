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
#define TPS 60

/* configuration for drawing. */
#define DRAW_FLY_R 0
#define DRAW_FLY_G 0
#define DRAW_FLY_B 0
#define DRAW_BG_R 190
#define DRAW_BG_G 190
#define DRAW_BG_B 190
#define DRAW_OBSTACLE_R 90
#define DRAW_OBSTACLE_G 0
#define DRAW_OBSTACLE_B 0
#define DRAW_GOAL_R 0
#define DRAW_GOAL_G 200
#define DRAW_GOAL_B 0
#define DRAW_FLY_SIZE 8
#define DRAW_GOAL_SIZE GA_GOAL_SIZE

/* configuration for genetic algorithm. */
#define GA_FLY_MAX_MOVES 90
#define GA_FLY_SPEED 7
#define GA_FLY_START_X (WIDTH / 2)
#define GA_FLY_START_Y (HEIGHT * 3 / 4)
#define GA_GOAL_SIZE 8
#define GA_MUTATION_RATE 5
#define GA_GENERATION_SIZE 500

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

static struct fly fly_create(struct fly const *parent);
static double fly_fitness(struct fly const *fly);
static void fly_move(struct fly *fly);
static int random_int(int lower_bound, int upper_bound);
static int clamp(int lower_bound, int n, int upper_bound);
static void reset_generation(void);

static struct fly flies[GA_GENERATION_SIZE];
static int generation_num = 1;

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

    reset_generation();

    while (running) {
        SDL_Event e;
        size_t i;
        bool generation_dead = true;

        while (SDL_PollEvent(&e) == 1) {
            if (e.type == SDL_QUIT)
                running = false;
        }

        /* game logic. */
        for (i = 0; i < GA_GENERATION_SIZE; ++i) {
            fly_move(&flies[i]);
            generation_dead = generation_dead && flies[i].dead;
        }

        if (generation_dead) {
            ++generation_num;
            reset_generation();
        }

        /* rendering. */
        SDL_SetRenderDrawColor(rend, DRAW_BG_R, DRAW_BG_G, DRAW_BG_B, 255);
        SDL_RenderClear(rend);

        SDL_SetRenderDrawColor(rend, DRAW_FLY_R, DRAW_FLY_G, DRAW_FLY_B, 255);
        for (i = 0; i < GA_GENERATION_SIZE; ++i) {
            SDL_Rect r;

            r.x = flies[i].x - DRAW_FLY_SIZE / 2;
            r.y = flies[i].y - DRAW_FLY_SIZE / 2;
            r.w = r.h = DRAW_FLY_SIZE;
            SDL_RenderFillRect(rend, &r);
        }

        SDL_RenderPresent(rend);
        SDL_Delay(1000 / TPS);
    }

    SDL_Quit();
    return 0;
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
    return 0.0;
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

    fly->x = clamp(0, fly->x + fly->moves_x[next_move], WIDTH);
    fly->y = clamp(0, fly->y + fly->moves_y[next_move], HEIGHT);
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

static void reset_generation(void)
{
    size_t i;
    double best_fitness = fly_fitness(&flies[0]);
    struct fly const *fittest_fly = &flies[0];

    /*
     * generation 1 is the first generation, so there is no point in resetting
     * to the fittest fly, since they are all the same.
     */
    if (generation_num == 1) {
        for (i = 0; i < GA_GENERATION_SIZE; ++i)
            flies[i] = fly_create(NULL);

        return;
    }

    for (i = 1; i < GA_GENERATION_SIZE; ++i) {
        double fitness = fly_fitness(&flies[i]);

        if (fitness > best_fitness) {
            fittest_fly = &flies[i];
            best_fitness = fitness;
        }
    }

    for (i = 0; i < GA_GENERATION_SIZE; ++i)
        flies[i] = fly_create(fittest_fly);
}
