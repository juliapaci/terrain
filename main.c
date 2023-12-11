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
    edge_map[WIDTH-1][HEIGHT-1] = map[WIDTH-1][HEIGHT-1];

    return edge_map;
}

// TODO: make better wat to save initital mouse coordinates when dragging
bool first = true;
int original_x = 0;
int original_y = 0;
void add_circle(List *objects, int radius) {
    if(IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
        if(first) {
            original_x = GetMouseX();
            original_y = GetMouseY();
            first = false;
        }
        DrawLine(original_x, original_y, GetMouseX(), GetMouseY(), GRAY);
    } else if(!first && IsMouseButtonReleased(MOUSE_BUTTON_RIGHT)){
        const int i = GetMouseX() - original_x;
        const int j = GetMouseY() - original_y;
        int mag = sqrt(i*i + j*j);
        if(mag == 0)
            mag = 1;
        list_enqueue(objects, (phys_Object) {
                (phys_Vector) {
                    i,
                    j,
                    mag
                },
                radius/5,
                radius,
                original_x,
                original_y
            });
        first = true;
    }
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
    List objects;

    // physics object radius
    int radius = 10;

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

        radius += GetMouseWheelMove()*2;
        if(radius < 5)
            radius = 5;
        else if(radius > 50)
            radius = 50;

        add_circle(&objects, radius);
        draw_objects(&objects);
        apply_physics(&objects);

        // debug stuff
        int size = list_size(&objects);
        DrawText(TextFormat("size: %d", size), 500, 500, 100, BLUE);

        EndDrawing();
    }


    UnloadTexture(perlin_noise);
    CloseWindow();
    free(map_edge_data);
    list_free(&objects);
    return 0;
}
