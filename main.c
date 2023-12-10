#include <stdlib.h>
#include <stdio.h>

#include <raylib.h>
#define RAYGUI_IMPLEMENTATION
#include <raygui.h>

#include "perlin.h"

#define DATA_SIZE 500
#define OCTAVE_AMOUNT 1
#define WIDTH 1920
#define HEIGHT 1080


// TODO: use hw accel with gpu for perlin stuff
unsigned char *get_perlin() {
    unsigned char *perlin_data = malloc(WIDTH*HEIGHT);
    if(!perlin_data)
        return NULL;
    for(int x = 0; x < WIDTH; x++) {
        for(int y = 0; y < HEIGHT; y++) {
            float val = 0;
            float freq = 1;
            float amp = 1;

            for(int i = 0; i < OCTAVE_AMOUNT; i++) {
                val += perlin(x * freq / DATA_SIZE, y * freq / DATA_SIZE) * amp;
                freq *= 2;
                amp /= 2;
            }

            val *= 1.2;

            if(val > 1)
                val = 1;
            else if(val < -1)
                val = -1;

            int colour = ((val + 1) * 0.5) * 255;
            perlin_data[y * WIDTH + x] = colour;
        }
    }

    return perlin_data;
}

bool **generate_map(unsigned char *perlin) {
    bool **map = (bool **)malloc(sizeof(bool *) * WIDTH*HEIGHT);

    for(int x = 0; x < WIDTH; x++) {
        map[x] = malloc(HEIGHT);
        for(int y = 0; y < HEIGHT; y++) {
            if(perlin[(WIDTH*y + x) + (GetMouseY()*WIDTH + GetMouseX())] < 255/2) {
                map[x][y] = 0;
                continue;
            }

            map[x][y] = 1;
        }
    }

    return map;
 }

void draw_map(bool **map) {
    for(int x = 0; x < WIDTH; x++) {
        for(int y = 0; y < HEIGHT; y++) {
            if(!map[x][y])
                continue;

            DrawRectangleRec(
                (Rectangle) {
                    x,
                    y,
                    1,
                    1
                },
                (Color) {
                    64,
                    64,
                    64,
                    255
                }
            );
        }
    }
}

int main(void) {
    unsigned char *perlin_data = get_perlin();
    if(perlin_data == NULL)
        return 1;
    bool **map_data = generate_map(perlin_data);
    if(map_data == NULL)
        return 1;

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    SetConfigFlags(FLAG_VSYNC_HINT); // just for me
    InitWindow(0, 0, "Terrain");

    Image perlin_noise_img = {
        perlin_data,
        WIDTH,
        HEIGHT,
        PIXELFORMAT_UNCOMPRESSED_GRAYSCALE,
        1
    };
    Texture2D perlin_noise = LoadTextureFromImage(perlin_noise_img);

    bool show_perlin = false;
    bool show_map = false;
    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(RAYWHITE);

        if(show_map)
            draw_map(map_data);
        if(show_perlin)
            DrawTexture(perlin_noise, 0, 0, WHITE);

        if(GuiButton((Rectangle) {10, 50, 200, 20}, "draw map from noise"))
            show_map = !show_map;
        if(GuiButton((Rectangle) {10, 10, 200, 20}, "draw perlin noise as texture"))
            show_perlin = !show_perlin;

        EndDrawing();
    }


    UnloadTexture(perlin_noise);
    CloseWindow();
    free(perlin_data);
    free(map_data);

    return 0;
}
