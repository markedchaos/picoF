#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "ssd1306.h"
#include "registry.h"
//static ssd1306_t disp;
#include "ssd1306_compat.h"

#define SCREEN_W 128
#define SCREEN_H 64
#define PADDLE_W 20
#define PADDLE_H 3
#define BALL_SIZE 2
#define BRICK_W 16
#define BRICK_H 5
#define BRICK_COLS 8
#define BRICK_ROWS 3
#define MAX_LEVEL 3
#define I2C_PORT i2c1


// Input buttons (adjust to your wiring)
#define BTN_LEFT 14
#define BTN_RIGHT 15
#define BTN_EXIT 16

typedef struct {
    int x, y;
    bool alive;
} Brick;

static Brick bricks[BRICK_ROWS][BRICK_COLS];
static int paddle_x;
static int ball_x, ball_y;
static int ball_dx, ball_dy;
static int score;
static int level;
static bool running;

static void draw_paddle() {
    ssd1306_fill_rect(paddle_x, SCREEN_H - 6, PADDLE_W, PADDLE_H, 1);
}

static void draw_ball() {
    ssd1306_fill_rect(ball_x, ball_y, BALL_SIZE, BALL_SIZE, 1);
}

static void draw_bricks() {
    for (int r = 0; r < BRICK_ROWS; r++) {
        for (int c = 0; c < BRICK_COLS; c++) {
            if (bricks[r][c].alive) {
                ssd1306_fill_rect(bricks[r][c].x, bricks[r][c].y, BRICK_W - 1, BRICK_H - 1, 1);
            }
        }
    }
}

static void init_bricks() {
    for (int r = 0; r < BRICK_ROWS; r++) {
        for (int c = 0; c < BRICK_COLS; c++) {
            bricks[r][c].x = c * BRICK_W;
            bricks[r][c].y = r * BRICK_H + 10;
            bricks[r][c].alive = true;
        }
    }
}

static void reset_ball_paddle() {
    paddle_x = (SCREEN_W - PADDLE_W) / 2;
    ball_x = SCREEN_W / 2;
    ball_y = SCREEN_H / 2;
    ball_dx = (rand() % 2) ? 1 : -1;
    ball_dy = -1;
}

static void draw_center_text(const char *text, int y) {
    int w = strlen(text) * 6;
    ssd1306_draw_string((SCREEN_W - w) / 2, y, text, 1, 0);
}

static void wait_for_button() {
    while (true) {
        if (!gpio_get(BTN_LEFT) || !gpio_get(BTN_RIGHT) || !gpio_get(BTN_EXIT)) break;
        sleep_ms(50);
    }
    sleep_ms(200);
}

static void show_level_screen() {
    ssd1306_clear();
    char buf[16];
    sprintf(buf, "LEVEL %d", level);
    draw_center_text(buf, 28);
    ssd1306_show();
    wait_for_button();
}

static void show_game_over(bool final_level) {
    ssd1306_clear();
    if (final_level) {
        // Twinkling stars celebration
        absolute_time_t end_time = make_timeout_time_ms(3000);
        while (!time_reached(end_time)) {
            ssd1306_clear();
            for (int i = 0; i < 20; i++) {
                int x = rand() % SCREEN_W;
                int y = rand() % SCREEN_H;
                ssd1306_draw_pixel(x, y, 1);
            }
            draw_center_text("YOU WIN!", 28);
            ssd1306_show();
            sleep_ms(200);
        }
    } else {
        draw_center_text("GAME OVER", 24);
        char buf[16];
        sprintf(buf, "SCORE: %d", score);
        draw_center_text(buf, 36);
        ssd1306_show();
        wait_for_button();
    }
}

static void update_ball() {
    ball_x += ball_dx;
    ball_y += ball_dy;

    // Wall collisions
    if (ball_x <= 0 || ball_x >= SCREEN_W - BALL_SIZE) ball_dx = -ball_dx;
    if (ball_y <= 0) ball_dy = -ball_dy;

    // Paddle collision
    if (ball_y >= SCREEN_H - 6 - BALL_SIZE &&
        ball_x + BALL_SIZE >= paddle_x &&
        ball_x <= paddle_x + PADDLE_W) {
        ball_dy = -ball_dy;
        // Add some horizontal variation
        if (ball_x < paddle_x + PADDLE_W / 3) ball_dx = -1;
        else if (ball_x > paddle_x + 2 * PADDLE_W / 3) ball_dx = 1;
    }

    // Brick collisions
    for (int r = 0; r < BRICK_ROWS; r++) {
        for (int c = 0; c < BRICK_COLS; c++) {
            Brick *b = &bricks[r][c];
            if (b->alive &&
                ball_x + BALL_SIZE > b->x &&
                ball_x < b->x + BRICK_W &&
                ball_y + BALL_SIZE > b->y &&
                ball_y < b->y + BRICK_H) {
                b->alive = false;
                ball_dy = -ball_dy;
                score += 10;
            }
        }
    }
}

static bool bricks_remaining() {
    for (int r = 0; r < BRICK_ROWS; r++)
        for (int c = 0; c < BRICK_COLS; c++)
            if (bricks[r][c].alive) return true;
    return false;
}

static void handle_input() {
    if (!gpio_get(BTN_LEFT) && paddle_x > 0) paddle_x -= 2;
    if (!gpio_get(BTN_RIGHT) && paddle_x < SCREEN_W - PADDLE_W) paddle_x += 2;
    if (!gpio_get(BTN_EXIT)) running = false;
}

static void game_loop() {
    running = true;
    score = 0;
    level = 1;

    while (running && level <= MAX_LEVEL) {
        init_bricks();
        reset_ball_paddle();
        show_level_screen();

        while (running) {
            handle_input();
            update_ball();

            if (ball_y > SCREEN_H) {
                running = false;
                break;
            }

            if (!bricks_remaining()) {
                level++;
                break;
            }

            ssd1306_clear();
            draw_paddle();
            draw_ball();
            draw_bricks();
            ssd1306_show();
            sleep_ms(10 + (MAX_LEVEL - level) * 5); // speed up with level
        }
    }

    show_game_over(level > MAX_LEVEL);
}

void run_brickout(void) {
    gpio_init(BTN_LEFT); gpio_set_dir(BTN_LEFT, GPIO_IN); gpio_pull_up(BTN_LEFT);
    gpio_init(BTN_RIGHT); gpio_set_dir(BTN_RIGHT, GPIO_IN); gpio_pull_up(BTN_RIGHT);
    gpio_init(BTN_EXIT); gpio_set_dir(BTN_EXIT, GPIO_IN); gpio_pull_up(BTN_EXIT);
    
    ssd1306_init(&disp, I2C_PORT, 0x3C, SCREEN_W, SCREEN_H);

    ssd1306_clear();
    draw_center_text("BRICK-OUT", 24);
    ssd1306_show();
    wait_for_button();

    game_loop();
}

REGISTER_PROGRAM(brickout, "Brick-Out", NULL);

