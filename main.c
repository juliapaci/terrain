#include <stdlib.h>
#include <stdio.h>

#define RAYGUI_IMPLEMENTATION
#include <raygui.h>
#include <raylib.h>

#include "perlin.h"
#include "physics.h"

#define THRESHOLD 255/2

bool **generate_map(unsigned char *perlin) {
    bool **map = malloc(sizeof(bool *) * WIDTH*HEIGHT);

    for(int x = 0; x < WIDTH; x++) {
        map[x] = malloc(HEIGHT);
        for(int y = 0; y < HEIGHT; y++) {
            if(perlin[(WIDTH*y + x) + (GetMouseY()*WIDTH + GetMouseX())] >= THRESHOLD) {
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

void add_circle(Array *objects) {
    array_push(objects, (phys_Object) {
            (phys_Vector) {1,1,1},
            5,
            GetMouseX(),
            GetMouseY()
        });
}

int main(void) {

    // precompute data
    unsigned char *perlin_data = get_perlin();
    if(perlin_data == NULL)
        return 1;
    bool **map_data = generate_map(perlin_data);
    if(map_data == NULL)
        return 1;
    bool **map_edge_data = edge_filter(map_data);
    if(map_edge_data == NULL)
        return 1;

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

    // generate textures from precomputed data
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

    // keep track of physics objects
    Array objects;
    array_init(&objects);

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

        if(IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
            add_circle(&objects);
        draw_objects(&objects);

        EndDrawing();
    }


    UnloadTexture(perlin_noise);
    CloseWindow();
    free(map_edge_data);

    return 0;
}
