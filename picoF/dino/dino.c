// Chrome Dino
// Buttons: GP8 jump, GP9 duck (hold), GP7 restart

#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "ssd1306.h"
#include "gfx.h"
#include "registry.h"
#include "hardware_init.h"

REGISTER_PROGRAM(dino, "Dino", NULL);

// ===== Pins ===== Now currently live in hardware/hardware_config.h
//#define I2C_PORT i2c1
//#define SDA_PIN  26
//#define SCL_PIN  27

//#define BTN_RESTART 7
//#define BTN_JUMP    8
//#define BTN_DUCK    9

// ===== Display =====
//static ssd1306_t disp;

// ===== Game tuning =====
#define OLED_W 128
#define OLED_H  64
#define FRAME_MS        33    // ~30 FPS
#define GROUND_Y        54
#define DINO_X          14
#define GRAVITY          1.2
#define JUMP_VEL       (-10)
#define INIT_SPEED_X     3
#define MAX_SPEED_X      7
#define BIRD_UNLOCK     250   // unlock pterodactyl after this score
#define ANIM_MS          90

// ===== Button handling =====
typedef struct { bool prev; bool cur; uint pin; } btn_t;
static btn_t b_jump    = {.prev=true,.cur=true,.pin=BTN_JUMP};
static btn_t b_duck    = {.prev=true,.cur=true,.pin=BTN_DUCK};
static btn_t b_restart = {.prev=true,.cur=true,.pin=BTN_RESTART};

static inline void btn_read(btn_t* b) { b->prev = b->cur; b->cur = gpio_get(b->pin); } // pull-ups: 1=idle
static inline bool pressed(const btn_t* b) { return (b->prev == false) && (b->cur == true); }
static inline bool held(const btn_t* b) { return b->cur == true; }

// ===== RNG =====
static uint32_t rng_state = 0xA2C2B3D5u;
static inline uint32_t xr() { uint32_t x=rng_state; x^=x<<13; x^=x>>17; x^=x<<5; return rng_state=x; }
static inline int rr(int a,int b) { uint32_t r=xr(); int span=(b-a+1); return a + (int)(r % (uint32_t)span); }

// ===== Sprites (rows of '.' and '#') =====
// Dino run 16x16
static const char* DINO_RUN_A[16] = {
"................",
"......####......",
".....######.....",
"....########....",
"...##########...",
"...##########...",
"..###########...",
"..###########...",
"..####..######..",
"..####..######..",
"..####..######..",
"..####..######..",
"..####..###.....",
"..######..##....",
".####..######...",
"......##........"
};
static const char* DINO_RUN_B[16] = {
"................",
"......####......",
".....######.....",
"....########....",
"...##########...",
"...##########...",
"..###########...",
"..###########...",
"..####..######..",
"..####..######..",
"..####..######..",
"..####..######..",
"..####..###.....",
"..######..##....",
"......######....",
".......##......."
};
// Dino duck 22x12
static const int DINO_DUCK_W = 22, DINO_DUCK_H = 12;
static const char* DINO_DUCK_A[12] = {
"......##########......",
".....############.....",
"....##############....",
"...################...",
"..##################..",
"..######..#########...",
"..######..#########...",
"..######..#########...",
"..######..######......",
"..######..######......",
"..#####....####.......",
"...##.................."
};
static const char* DINO_DUCK_B[12] = {
"......##########......",
".....############.....",
"....##############....",
"...################...",
"..##################..",
"..######..#########...",
"..######..#########...",
"..######..#########...",
"..######..######......",
"..######..######......",
"...####..#####........",
"....##..............."
};
// Cactus small 8x16
static const char* CACTUS_S_8x16[16] = {
"..##....",
"..##....",
"..##....",
"..##....",
"######..",
"..##....",
"..##....",
"..##....",
"..##....",
"..##.##.",
"######..",
"..##....",
"..##....",
"..##....",
"..##....",
"..##...."
};
// Cactus tall 12x18
static const int CACTUS_L_W=12, CACTUS_L_H=18;
static const char* CACTUS_L_12x18[18] = {
"...####.....",
"...####.....",
"...####.....",
"...####.....",
"#########...",
"...####.....",
"...####..##.",
"...####..##.",
"...####.....",
"...####.....",
"...####.....",
"...####.....",
"...####.....",
"#########...",
"...####.....",
"...####.....",
"...####.....",
"...####....."
};
// Bird 16x8
static const char* BIRD_A_16x8[8] = {
"........#.......",
".......###......",
"############....",
"........###.....",
"..........##....",
"...........#....",
"................",
"................"
};
static const char* BIRD_B_16x8[8] = {
"........#.......",
".......###......",
"############....",
"......##........",
".....##.........",
"....##..........",
"................",
"................"
};

// ===== Obstacles =====
typedef enum { OBS_CACTUS_S, OBS_CACTUS_L, OBS_BIRD } obs_type_t;
typedef struct {
    bool active; int x, y, w, h; obs_type_t type;
} obstacle_t;
#define MAX_OBS 3
static obstacle_t obs[MAX_OBS];

// ===== Game state =====
static bool jumping=false, ducking=false, game_over=false;
static int dino_y = GROUND_Y, vel_y = 0;
static int speed_x = INIT_SPEED_X;
static uint32_t score=0, hi_score=0;

// ===== Helpers =====
static void draw_ground(void) {
    for (int x=0; x<OLED_W; x+=4) { gfx_plot(x, GROUND_Y, true); gfx_plot(x+1, GROUND_Y, true); }
}
static void draw_clouds(uint32_t t) {
    int c1 = (int)(OLED_W - ((t/6) % (OLED_W+30)));
    int c2 = (int)(OLED_W/2 - ((t/9) % (OLED_W+30)));
    for (int dx=0; dx<12; dx++) {
        gfx_plot(c1+dx, 12 + ((dx%4)==0), true);
        gfx_plot(c2+dx, 20 + ((dx%5)==0), true);
    }
}
static void draw_dino(uint32_t t_ms) {
    bool alt = ((t_ms / ANIM_MS) % 2) == 0;
    if (!jumping && ducking) {
        gfx_sprite_rows(DINO_X, dino_y - DINO_DUCK_H, DINO_DUCK_W, DINO_DUCK_H, alt ? DINO_DUCK_A : DINO_DUCK_B);
    } else {
        gfx_sprite_rows(DINO_X, dino_y - 16, 16, 16, alt ? DINO_RUN_A : DINO_RUN_B);
    }
}
static void draw_obstacle(const obstacle_t* o, uint32_t t_ms) {
    if (!o->active) return;
    switch (o->type) {
        case OBS_CACTUS_S:
            gfx_sprite_rows(o->x, GROUND_Y - 16, 8, 16, CACTUS_S_8x16);
            break;
        case OBS_CACTUS_L:
            gfx_sprite_rows(o->x, GROUND_Y - CACTUS_L_H, CACTUS_L_W, CACTUS_L_H, CACTUS_L_12x18);
            break;
        case OBS_BIRD: {
            bool wing = ((t_ms/120)%2)==0;
            gfx_sprite_rows(o->x, o->y - 8, 16, 8, wing ? BIRD_A_16x8 : BIRD_B_16x8);
            } break;
    }
}
static bool aabb(int ax,int ay,int aw,int ah,int bx,int by,int bw,int bh){
    return (ax < bx + bw) && (ax + aw > bx)
    && (ay < by + bh) && (ay + ah > by);
}

static bool dino_hit(const obstacle_t* o) {
    int dw, dh, dy;
    if (!jumping && ducking) {
        dw = DINO_DUCK_W; dh = DINO_DUCK_H; dy = dino_y - DINO_DUCK_H;
    } else {
        dw = 16; dh = 16; dy = dino_y - 16;
    }
    return aabb(DINO_X, dy, dw, dh, o->x, o->y - o->h, o->w, o->h);
}

static void update_obstacles(void) {
    for (int i = 0; i < MAX_OBS; i++) {
        if (obs[i].active) {
            obs[i].x -= speed_x;
            if (obs[i].x + obs[i].w < 0) obs[i].active = false;
        }
    }
    // spawn new if space
    for (int i = 0; i < MAX_OBS; i++) {
        if (!obs[i].active) {
            bool space = true;
            for (int j = 0; j < MAX_OBS; j++) {
                if (obs[j].active && obs[j].x > OLED_W - 30) { space = false; break; }
            }
            if (!space) break;
            // choose type
            obs_type_t t;
            if (score > BIRD_UNLOCK && rr(0, 4) == 0) {
                t = OBS_BIRD;
            } else {
                t = (rr(0, 1) == 0) ? OBS_CACTUS_S : OBS_CACTUS_L;
            }
            obs[i].type = t;
            obs[i].active = true;
            obs[i].x = OLED_W + rr(0, 20);
            switch (t) {
                case OBS_CACTUS_S:
                    obs[i].w = 8; obs[i].h = 16; obs[i].y = GROUND_Y;
                    break;
                case OBS_CACTUS_L:
                    obs[i].w = CACTUS_L_W; obs[i].h = CACTUS_L_H; obs[i].y = GROUND_Y;
                    break;
                case OBS_BIRD:
                    obs[i].w = 16; obs[i].h = 8; obs[i].y = (rr(0, 1) == 0) ? (GROUND_Y - 8) : (GROUND_Y - 20);
                    break;
            }
            break;
        }
    }
}

static void draw_scores(void) {
    char buf[16];
    snprintf(buf, sizeof(buf), "%lu", (unsigned long)score);
    gfx_text5x7(OLED_W - (strlen(buf) * 6) - 2, 2, buf, true);
    snprintf(buf, sizeof(buf), "HI %lu", (unsigned long)hi_score);
    gfx_text5x7(2, 2, buf, true);
}

static void reset_game(void) {
    jumping = false;
    ducking = false;
    game_over = false;
    dino_y = GROUND_Y;
    vel_y = 0;
    speed_x = INIT_SPEED_X;
    score = 0;
    for (int i = 0; i < MAX_OBS; i++) obs[i].active = false;
}

void run_dino(void) {
//    stdio_init_all();
//    i2c_init(I2C_PORT, 400 * 1000);
//    gpio_set_function(SDA_PIN, GPIO_FUNC_I2C);
//    gpio_set_function(SCL_PIN, GPIO_FUNC_I2C);
//    gpio_pull_up(SDA_PIN);
//    gpio_pull_up(SCL_PIN);

//    gpio_init(BTN_JUMP); gpio_set_dir(BTN_JUMP, GPIO_IN); gpio_pull_down(BTN_JUMP);
//    gpio_init(BTN_DUCK); gpio_set_dir(BTN_DUCK, GPIO_IN); gpio_pull_down(BTN_DUCK);
//    gpio_init(BTN_RESTART); gpio_set_dir(BTN_RESTART, GPIO_IN); gpio_pull_down(BTN_RESTART);

//    disp.external_vcc = false;
//old-    ssd1306_init(&disp, OLED_W, OLED_H, 0x3C, I2C_PORT);
//    ssd1306_init(&disp, I2C_PORT, 0x3C, OLED_W, OLED_H);
    hardware_init();
    gfx_init(&disp);

    reset_game();

    uint32_t t_ms = 0;
    uint32_t anim_t = 0;

    while (true) {
        sleep_ms(FRAME_MS);
        t_ms += FRAME_MS;
        anim_t += FRAME_MS;

        btn_read(&b_jump);
        btn_read(&b_duck);
        btn_read(&b_restart);

        if (!game_over) {
            if (pressed(&b_jump) && !jumping) { jumping = true; vel_y = JUMP_VEL; ducking = false; }
            ducking = held(&b_duck) && !jumping;

            if (jumping) {
                dino_y += vel_y;
                vel_y += GRAVITY;
                if (dino_y >= GROUND_Y) { dino_y = GROUND_Y; vel_y = 0; jumping = false; }
            }

            update_obstacles();

            for (int i = 0; i < MAX_OBS; i++) {
                if (obs[i].active && dino_hit(&obs[i])) {
                    game_over = true;
                    if (score > hi_score) hi_score = score;
                    break;
                }
            }

            score++;
            if ((score % 150) == 0 && speed_x < MAX_SPEED_X) speed_x++;

            // Render
            gfx_clear();
            draw_clouds(t_ms);
            draw_ground();
            draw_dino(anim_t);
            for (int i = 0; i < MAX_OBS; i++) draw_obstacle(&obs[i], anim_t);
            draw_scores();
            gfx_show();
        } else {
            gfx_clear();
            draw_ground();
            draw_dino(anim_t);
            for (int i = 0; i < MAX_OBS; i++) draw_obstacle(&obs[i], anim_t);
            draw_scores();
            gfx_text5x7(37, 22, "GAME OVER", true);
            gfx_text5x7(25, 38, "PRESS RESTART", true);
            gfx_show();

            if (held(&b_restart)) {
                reset_game();
                sleep_ms(150);
            }
        }
    }
}



