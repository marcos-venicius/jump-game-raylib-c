#include <string.h>
#include <time.h>
#include <raylib.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#define WIDTH 800
#define HEIGHT 800
#define HEX(c) ((Color){.r = c >> (3 * 8) & 0xFF, .g = c >> (2 * 8) & 0xFF, .b = c >> (1 * 8) & 0xFF, .a = c >> (0 * 8) & 0xFF})

typedef struct {
    Rectangle dim;
    Color color;
    Vector2 vel;
} Object;

typedef struct {
    int width;
    int height;
    int font_size;
    char *text;
} Text;

static bool collided(Object a, Object b) {
    bool overlapX = a.dim.x + a.dim.width >= b.dim.x && b.dim.x + b.dim.width >= a.dim.x;
    bool overlapY = a.dim.y + a.dim.height >= b.dim.y && b.dim.y + b.dim.height >= a.dim.y;

    return overlapX && overlapY;
}

static Text create_text(char *text, int font_size) {
    int width = MeasureText(text, font_size);

    Text object = {
        .width = width,
        .height = font_size,
        .font_size = font_size,
        .text = NULL
    };

    object.text = malloc(sizeof(char) * (strlen(text) + 1));

    if (object.text != NULL) {
        strcpy(object.text, text);
    }

    return object;
}

static void update_text(Text *object, char *to) {
    int width = MeasureText(to, object->font_size);

    char *new_text = realloc(object->text, (strlen(to) + 1));

    object->width = width;

    if (new_text != NULL) {
        object->text = new_text;
        strcpy(object->text, to);
    }
}

void game(void) {
    bool running = true;
    double previousTime = GetTime();
    double currentTime = 0;
    double updateDrawTime = 0;
    double waitTime = 0;
    int targetFPS = 120;
    unsigned int points = 0;

    Text points_text = create_text("0", 50);

    Vector2 points_text_pos = {
        .x = ((float)WIDTH / 2) - points_text.width / 2.f,
        .y = ((float)HEIGHT / 2) - points_text.height / 2.f
    };

    SetRandomSeed((unsigned int)time(NULL));

    Object player = (Object){
        .dim = {
            .x = GetRandomValue(0, WIDTH - 20),
            .y = 0,
            .width = 20,
            .height = 20
        },
        .vel = { .x = 0, .y = 1 },
        .color = GREEN
    };

    Object obstacle = (Object){
        .dim = {
            .x = 0,
            .y = HEIGHT - 20,
            .width = 0.2 * WIDTH,
            .height = 20
        },
        .vel = { .x = 5, .y = 0 },
        .color = WHITE
    };

    size_t frame_id = 0;
    size_t last_frame_id = 0;
    float gravity = .8;

    while (!WindowShouldClose() && running) {
        ++frame_id;
        PollInputEvents();

        if (IsKeyPressed(KEY_SPACE) && gravity >= 0) {
            gravity = -5;
            last_frame_id = frame_id;
        }

        if (IsKeyDown(KEY_RIGHT)) {
            player.vel.x += 0.05;
        } if (IsKeyDown(KEY_LEFT)) {
            player.vel.x -= 0.05;
        }

        if (targetFPS < 0) targetFPS = 0;

        // rules update

        // gravity
        if (gravity < .8) gravity += (frame_id - last_frame_id) * 0.001;

        // player
        if (player.vel.x > 0) player.vel.x -= 0.01;
        else if (player.vel.x < 0) player.vel.x += 0.01;

        player.dim.x += player.vel.x;
        player.dim.y += player.vel.y + gravity;

        if (player.dim.y < 0) {
            player.dim.y = 0;
            gravity = .8;
        }

        if (player.dim.x + player.dim.width > WIDTH) {
            player.dim.x = WIDTH - player.dim.width;
            player.vel.x *= -1;
        } else if (player.dim.x < 0) {
            player.dim.x = 0;
            player.vel.x *= -1;
        }

        // obstacle
        obstacle.dim.x += obstacle.vel.x;

        if (obstacle.dim.x >= WIDTH - obstacle.dim.width) {
            obstacle.dim.x = WIDTH - obstacle.dim.width;
            obstacle.vel.x = -obstacle.vel.x;
        } else if (obstacle.dim.x < 0) {
            obstacle.dim.x = 0;
            obstacle.vel.x *= -1;
        }

        BeginDrawing();

        ClearBackground(BLACK);

        DrawText(points_text.text, points_text_pos.x, points_text_pos.y, 50, HEX(0x282828FF));

        DrawRectangle(player.dim.x, player.dim.y, player.dim.width, player.dim.height, player.color);
        DrawRectangle(obstacle.dim.x, obstacle.dim.y, obstacle.dim.width, obstacle.dim.height, obstacle.color);

        EndDrawing();

        currentTime = GetTime();
        updateDrawTime = currentTime - previousTime;

        if (targetFPS > 0) {
            waitTime = (1.0f / (float)targetFPS) - updateDrawTime;
            if (waitTime > 0.0) {
                WaitTime((float)waitTime);
                currentTime = GetTime();
            }
        }

        previousTime = currentTime;

        if (collided(player, obstacle)) {
            ++points;

            char text[20] = {0};

            sprintf(text, "%u", points);

            update_text(&points_text, text);

            points_text_pos.x = ((float)WIDTH / 2) - points_text.width / 2.f;
            points_text_pos.y = ((float)HEIGHT / 2) - points_text.height / 2.f;

            if (player.vel.x == 0) player.vel.x = obstacle.vel.x;
            else player.vel.x = obstacle.vel.x + player.vel.x;

            gravity = -10;

            last_frame_id = frame_id;
        } else if (player.dim.y + player.dim.height > obstacle.dim.y) {
            running = false;
        }
    }

    if (!running) {
        Text game_end_message = create_text("Game End", 30);
        Vector2 game_end_message_pos = {.x = WIDTH / 2.f - game_end_message.width / 2.f, .y = HEIGHT / 2.f - points_text.height - game_end_message.height};

        Text info_message = create_text("Press \"q\" to quit and \"r\" to restart", 20);
        Vector2 info_message_pos = {.x = WIDTH / 2.f - info_message.width / 2.f, .y = HEIGHT / 2.f + points_text.height + info_message.height};

        while (!WindowShouldClose()) {
            PollInputEvents();

            if (IsKeyPressed(KEY_Q)) {
                break;
            }

            if (IsKeyPressed(KEY_R)) {
                free(points_text.text);
                free(game_end_message.text);
                free(info_message.text);

                game();
                break;
            }

            BeginDrawing();

            ClearBackground(BLACK);

            DrawText(points_text.text, points_text_pos.x, points_text_pos.y, 50, HEX(0x282828FF));
            DrawText(game_end_message.text, game_end_message_pos.x, game_end_message_pos.y, 30, HEX(0xFF2828FF));
            DrawText(info_message.text, info_message_pos.x, info_message_pos.y, 20, HEX(0xFFFFFFFF));

            EndDrawing();

            currentTime = GetTime();
            updateDrawTime = currentTime - previousTime;

            if (targetFPS > 0) {
                waitTime = (1.0f / (float)targetFPS) - updateDrawTime;
                if (waitTime > 0.0) {
                    WaitTime((float)waitTime);
                    currentTime = GetTime();
                }
            }

            previousTime = currentTime;
        }
    }
}

int main(void) {
    // Should be the first call
    InitWindow(WIDTH, HEIGHT, "JumpGame");

    game();

    CloseWindow();

    return 0;
}
