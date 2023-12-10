#include <stdlib.h>
#include <stdio.h>

#include <raylib.h>
#define RAYGUI_IMPLEMENTATION
#include <raygui.h>

#include "perlin.h"

#define DATA_SIZE 500
#define OCTAVE_AMOUNT 12
#define WIDTH 1920
#define HEIGHT 1080

#define THRESHOLD 255/2


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

// TODO: better way than a param for different types of maps
bool **generate_map(unsigned char *perlin, const bool edge) {
    bool **map = malloc(sizeof(bool *) * WIDTH*HEIGHT);

    for(int x = 0; x < WIDTH; x++) {
        map[x] = malloc(HEIGHT);
        for(int y = 0; y < HEIGHT; y++) {
            if((!edge && perlin[(WIDTH*y + x) + (GetMouseY()*WIDTH + GetMouseX())] >= THRESHOLD)
                    || (edge && perlin[(WIDTH*y + x) + (GetMouseY()*WIDTH + GetMouseX())] == THRESHOLD)) { // this edge optimisation only works because of specific interpolation (although most (prob all but none) will work)
                map[x][y] = 0;
                continue;
            }

            map[x][y] = 1;
        }
    }

    return map;
}

bool **edge_filter(bool **map) {
    bool **edge_map = malloc(sizeof(bool *) * WIDTH*HEIGHT);

    for(int x = 1; x < WIDTH-1; x++) {
        edge_map[x] = malloc(HEIGHT);
        for(int y = 1; y < HEIGHT-1; y++) {
            if(!map[x][y]) {
                edge_map[x][y] = 0;
                continue;
            }

            if(!map[x+1][y] || !map[x-1][y] || !map[x][y+1] || !map[x][y-1]) {
                edge_map[x][y] = 1;
                continue;
            }

            edge_map[x][y] = 0;
        }
    }

    edge_map[0] = malloc(HEIGHT);
    edge_map[0][0] = map[0][0];
    edge_map[WIDTH-1] = malloc(HEIGHT);
    edge_map[WIDTH-1][HEIGHT-1] = edge_map[WIDTH-1][HEIGHT-1];

    return edge_map;
}


int main(void) {
    unsigned char *perlin_data = get_perlin();
    if(perlin_data == NULL)
        return 1;
    bool **map_data = generate_map(perlin_data, false);
    if(map_data == NULL)
        return 1;
    bool **map_edge_data_approx = generate_map(perlin_data, true);
    if(map_edge_data_approx == NULL)
        return 1;
    // bool **map_edge_data = edge_filter(map_edge_data_approx);
    bool **map_edge_data = edge_filter(map_data);
    free(map_edge_data_approx);

    unsigned char edge_arr[WIDTH*HEIGHT] = {0};
    for(int x = 0; x < WIDTH; x++)
        for(int y = 0; y < HEIGHT; y++)
            edge_arr[y*WIDTH + x] = !map_edge_data[x][y] * 200;

    unsigned char map_gray[WIDTH*HEIGHT] = {0};
    for(int x = 0; x < WIDTH; x++)
        for(int y = 0; y < HEIGHT; y++)
            map_gray[y*WIDTH + x] = !map_data[x][y] * 200;
    free(map_data);

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
    free(perlin_data);

    Image map_img = {
        map_gray,
        WIDTH,
        HEIGHT,
        PIXELFORMAT_UNCOMPRESSED_GRAYSCALE,
        1,
    };
    Texture2D map = LoadTextureFromImage(map_img);

    Image edge_map_img = {
        edge_arr,
        WIDTH,
        HEIGHT,
        PIXELFORMAT_UNCOMPRESSED_GRAYSCALE,
        1,
    };
    Texture2D edge_map = LoadTextureFromImage(edge_map_img);

    bool show_perlin = false;
    bool show_map = false;
    bool show_edge = false;
    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(RAYWHITE);

        if(show_map)
            DrawTexture(map, 0, 0, WHITE);
        if(show_perlin)
            DrawTexture(perlin_noise, 0, 0, WHITE);
        if(show_edge)
            DrawTexture(edge_map, 0, 0, WHITE);

        if(GuiButton((Rectangle) {10, 10, 200, 20}, "draw perlin noise as texture"))
            show_perlin = !show_perlin;
        if(GuiButton((Rectangle) {10, 50, 200, 20}, "draw map from noise"))
            show_map = !show_map;
        if(GuiButton((Rectangle) {10, 90, 200, 20}, "draw edge data"))
            show_edge = !show_edge;

        // if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            // light();

        EndDrawing();
    }


    UnloadTexture(perlin_noise);
    CloseWindow();
    free(map_edge_data);

    return 0;
}
