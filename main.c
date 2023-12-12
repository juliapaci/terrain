#include <stdlib.h>
#include <stdio.h>

#define RAYGUI_IMPLEMENTATION
#include <raygui.h>
#include <raylib.h>

#include "perlin.h"
#include "physics.h"
#include "quadtree.h"

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

// TODO: could prob use a shader for this
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
        DrawCircle(original_x, original_y, radius, GRAY);
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

        if(list_size(objects) > LIST_CAP)
            list_dequeue(objects);

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
    int radius = 8;

    // quad tree
    QuadTreeNode quadtree;

    bool show_menu = true;
    bool show_perlin = false;
    bool show_map = false;
    bool show_edge = false;
    bool show_vec = false;
    bool show_quadtree = false;
    bool pause = false;

    const int button_width = 200;
    const int button_height = 20;
    while (!WindowShouldClose()) {
        int sw = GetScreenWidth();
        int sh = GetScreenHeight();

        // maybe pause too?
        init_tree(&quadtree);
        generate_tree(&quadtree, &objects);
        if(!pause) {
            apply_physics(&objects);
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);

        if(show_map)
            DrawTexture(map, 0, 0, WHITE);
        if(show_perlin)
            DrawTexture(perlin_noise, 0, 0, WHITE);
        if(show_edge)
            DrawTexture(edge_map, 0, 0, WHITE);
        if(show_quadtree)
            draw_tree(&quadtree);

        // menu (buttons & stuff)
        if(IsKeyPressed(KEY_M))
            show_menu = !show_menu;
        if(show_menu) {
            if(GuiButton((Rectangle) {10, 10, button_width, button_height}, "draw perlin noise"))
                show_perlin = !show_perlin;
            if(GuiButton((Rectangle) {10, 50, button_width, button_height}, "draw map from noise"))
                show_map = !show_map;
            if(GuiButton((Rectangle) {10, 90, button_width, button_height}, "draw edge data"))
                show_edge = !show_edge;
            if(GuiButton((Rectangle) {10, 130, button_width, button_height}, "draw object vectors (unfiltered)"))
                show_vec = !show_vec;
            if(GuiButton((Rectangle) {10, 170, button_width, button_height}, "visualise quadtree"))
                show_quadtree = !show_quadtree;
            if(GuiButton((Rectangle) {sw-50, 10, 30, 30}, pause ? ">" : "| |") || IsKeyPressed(KEY_SPACE))
                pause = !pause;

            // debug stuff
            DrawText(TextFormat("FPS: %d", GetFPS()), sw-250, 10, 30, BLUE);
            int size = list_size(&objects);
            DrawText(TextFormat("amount: %d", size), sw-250, 50, 30, BLUE);
            DrawText(TextFormat("Obj radius: %d", radius), sw-250, 90, 30, BLUE);
        }

        radius += GetMouseWheelMove()*2;
        if(radius < 5)
            radius = 5;
        else if(radius > 50)
            radius = 50;

        add_circle(&objects, radius);
        draw_objects(&objects, show_vec);

        EndDrawing();
    }


    UnloadTexture(perlin_noise);
    CloseWindow();
    free(map_edge_data);
    list_free(&objects);
    return 0;
}
