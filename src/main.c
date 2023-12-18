#define GLSL_VERSION 330

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

#define RAYGUI_IMPLEMENTATION
#include <raygui.h>
#include <raylib.h>

#include "perlin.h"
#include "physics.h"
#include "quadtree.h"
#include "shadow.h"

#define THRESHOLD 255/2

bool **generate_map(unsigned char *perlin) {
    bool **map = malloc(sizeof(bool *) * WIDTH*HEIGHT);

    for(int x = 0; x < WIDTH; x++) {
        map[x] = malloc(HEIGHT);
        for(int y = 0; y < HEIGHT; y++)
            map[x][y] = perlin[(WIDTH*y + x)] >= THRESHOLD;
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

            edge_map[x][y] = !map[x+1][y] || !map[x-1][y] || !map[x][y+1] || !map[x][y-1];
        }
    }

    edge_map[0] = malloc(HEIGHT);
    edge_map[0][0] = map[0][0];
    edge_map[WIDTH-1] = malloc(HEIGHT);
    edge_map[WIDTH-1][HEIGHT-1] = map[WIDTH-1][HEIGHT-1];

    return edge_map;
}

// TODO: fix segfault
void free_map(bool ***data) {
    for(int i = 0; i < WIDTH; i++) {
        free(*data[i]);
    }
    free(*data);
}

// TODO: make better wat to save initital mouse coordinates when dragging
bool add_first= true;
int add_original_x = 0;
int add_original_y = 0;
void add_circle(List *objects, int radius) {
    if(IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
        if(add_first) {
            add_original_x = GetMouseX();
            add_original_y = GetMouseY();
            add_first = false;
        }

        DrawLine(add_original_x, add_original_y, GetMouseX(), GetMouseY(), GRAY);
        DrawCircle(add_original_x, add_original_y, radius, GRAY);
    } else if(IsMouseButtonReleased(MOUSE_BUTTON_RIGHT)){
        const int i = GetMouseX() - add_original_x;
        const int j = GetMouseY() - add_original_y;
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
                add_original_x,
                add_original_y
            });

        if(list_size(objects) > LIST_CAP)
            list_dequeue(objects);

        add_first = true;
    }
}

bool sel_first = true;
int sel_original_x = 0;
int sel_original_y = 0;
void select_objects(List *objects) {
    if(IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        if(sel_first) {
            sel_original_x = GetMouseX();
            sel_original_y = GetMouseY();
            sel_first = false;
        }

        // TODO: better way of drawing this rectangle
        DrawLine(sel_original_x, sel_original_y, sel_original_x, GetMouseY(), GRAY);
        DrawLine(sel_original_x, sel_original_y, GetMouseX(), sel_original_y, GRAY);
        DrawLine(GetMouseX(), sel_original_y, GetMouseX(), GetMouseY(), GRAY);
        DrawLine(sel_original_x, GetMouseY(), GetMouseX(), GetMouseY(), GRAY);
    } else if(IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
        sel_first = true;
    }
}

int main(void) {
    pthread_t perlin_thread;

    // precompute data
    unsigned char *perlin_data;
    if(pthread_create(&perlin_thread, NULL, get_perlin, &(get_perlin_args) {0, 0, 1}))
        return 1;
    if(pthread_join(perlin_thread, &perlin_data))
        return 1;
    if(perlin_data == NULL)
        return 1;
    bool **map_data = generate_map(perlin_data);
    if(map_data == NULL)
        return 1;
    bool **map_edge_data = edge_filter(map_data);
    if(map_edge_data == NULL)
        return 1;

    unsigned char edge_arr[WIDTH*HEIGHT];
    for(int x = 0; x < WIDTH; x++)
        for(int y = 0; y < HEIGHT; y++)
            edge_arr[y*WIDTH + x] = !map_edge_data[x][y] * 200;

    unsigned char map_gray[WIDTH*HEIGHT] = {0};
    for(int x = 0; x < WIDTH; x++)
        for(int y = 0; y < HEIGHT; y++)
            map_gray[y*WIDTH + x] = !map_data[x][y] * 200;

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

    // camera speed
    float move_scalar = 10;

    // octave amount for perlin noise
    float octave_amount = 1;

    // root quad tree node
    QuadTreeNode quadtree;

    // light settings
    float light_size = 300;
    float interval = 4;

    // camera
    Camera2D camera = {
        (Vector2) {
            0,
            0
        },
        (Vector2) {
            0,
            0
        },
        0.0f,
        1.0f
    };

    // menu button toggles
    bool show_menu = true;
    bool show_perlin = false;
    bool show_map = false;
    bool show_edge = false;
    bool show_vec = false;
    bool show_quadtree = false;
    bool pause = false;

    const int button_width = 200;
    const int button_height = 20;
    const int slider_width = 200;
    const int slider_height = 25;
    while (!WindowShouldClose()) {
        int sw = GetScreenWidth();
        int sh = GetScreenHeight();

        if(!pause) {
            init_tree(&quadtree);
            generate_tree(&quadtree, &objects);
            apply_physics(&objects);

            if(show_edge)
                collide_physics(&objects, map_edge_data, show_edge);
            else if(show_map)
                collide_physics(&objects, map_data, show_map);
        }

        // camera stuff
        const float old_y = camera.target.y;
        const float old_x = camera.target.x;
        const float old_r = camera.rotation;
        const float old_z = camera.zoom;
        camera.target.y += move_scalar*(IsKeyDown(KEY_S) - IsKeyDown(KEY_W));
        camera.target.x += move_scalar*(IsKeyDown(KEY_D) - IsKeyDown(KEY_A));
        camera.rotation += IsKeyDown(KEY_E) - IsKeyDown(KEY_Q);
        camera.zoom += !IsMouseButtonDown(MOUSE_BUTTON_RIGHT)*((float)GetMouseWheelMove()*0.05f);
        if(IsKeyDown(KEY_R)) {
            camera.rotation = 0;
            camera.zoom = 1;
        }

        // regen chunks TODO: fix multithreading to make it work parallel not concurrently
        if(camera.target.y != old_y || camera.target.x != old_x || camera.rotation != old_r || camera.zoom != old_z) {
            // regen perlin noise
            if(pthread_create(&perlin_thread, NULL, get_perlin,
                    &(get_perlin_args) {
                            camera.target.x,
                            camera.target.y,
                            (unsigned int) octave_amount
                        }
                    ))
                return 1;
            if(pthread_join(perlin_thread, &perlin_data))
                    return 1;
            if(perlin_data == NULL)
                return 1;
            UnloadTexture(perlin_noise);
            perlin_noise_img.data = perlin_data;
            perlin_noise = LoadTextureFromImage(perlin_noise_img);

            // regen map
            if(pthread_create(&perlin_thread, NULL, generate_map, perlin_data))
                return 1;
            if(pthread_join(perlin_thread, &map_data))
                return 1;
            if(map_data == NULL)
                return 1;
            unsigned char map_gray[WIDTH*HEIGHT] = {0};
            for(int x = 0; x < WIDTH; x++)
                for(int y = 0; y < HEIGHT; y++)
                    map_gray[y*WIDTH + x] = !map_data[x][y] * 200;
            UnloadTexture(map);
            map_img.data = map_gray;
            map = LoadTextureFromImage(map_img);

            // regen edge map
            if(pthread_create(&perlin_thread, NULL, edge_filter, map_data))
                return 1;
            if(pthread_join(perlin_thread, &map_edge_data))
                return 1;
            if(map_edge_data == NULL)
                return 1;

            unsigned char edge_arr[WIDTH*HEIGHT];
            for(int x = 0; x < WIDTH; x++)
                for(int y = 0; y < HEIGHT; y++)
                    edge_arr[y*WIDTH + x] = !map_edge_data[x][y] * 200;
            edge_map_img.data = edge_arr;
            edge_map = LoadTextureFromImage(edge_map_img);

            free(perlin_data);
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);
        BeginMode2D(camera);

        if(show_edge)
            DrawTexture(edge_map, camera.target.x, camera.target.y, WHITE);
        else if(show_map)
            DrawTexture(map, camera.target.x, camera.target.y, WHITE);
        else if(show_perlin)
            DrawTexture(perlin_noise, camera.target.x, camera.target.y, WHITE);
        if(show_quadtree) {
            EndMode2D();
            draw_tree(&quadtree);
            BeginMode2D(camera);
        }
        EndMode2D();

        if(IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            if(show_edge)
                cast(GetMouseX(), GetMouseY(), light_size, interval, map_edge_data);
            else if(show_map)
                cast(GetMouseX(), GetMouseY(), light_size, interval, map_data);
        }

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
            if(GuiButton((Rectangle) {10, 170, button_width, button_height}, "clear objects"))
                while(list_size(&objects) > 0)
                    list_dequeue(&objects);
            if(GuiButton((Rectangle) {10, 210, button_width, button_height}, "visualise quadtree"))
                show_quadtree = !show_quadtree;
            GuiSlider((Rectangle) {65, 250, slider_width, slider_height}, "move speed", TextFormat("%.0f", move_scalar), &move_scalar, 5, 500);
            GuiSlider((Rectangle) {80, 290, slider_width, slider_height}, "octave amount", TextFormat("%.0f", octave_amount), &octave_amount, 1, 12);
            GuiSlider((Rectangle) {65, 330, slider_width, slider_height}, "light radius", TextFormat("%.0f", light_size), &light_size, 0, 1000);
            GuiSlider((Rectangle) {47, 370, slider_width, slider_height}, "interval", TextFormat("%.0f", interval), &interval, 1, 360);
            if(GuiButton((Rectangle) {sw-50, 10, 30, 30}, pause ? ">" : "| |") || IsKeyPressed(KEY_SPACE))
                pause = !pause;

            // debug stuff
            DrawText(TextFormat("FPS: %d", GetFPS()), sw-250, 10, 30, BLUE);
            int size = list_size(&objects);
            DrawText(TextFormat("amount: %d", size), sw-250, 50, 30, BLUE);
            DrawText(TextFormat("Obj radius: %d", radius), sw-250, 90, 30, BLUE);
        }

        if(IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
            radius += GetMouseWheelMove()*2;
            if(radius < 5)
                radius = 5;
            else if(radius > 50)
                radius = 50;
        }

        add_circle(&objects, radius);
        draw_objects(&objects, show_vec);
        select_objects(&objects);
        if(IsKeyPressed(KEY_BACKSPACE) || IsKeyPressed(KEY_DELETE))
            list_dequeue(&objects);

        EndDrawing();
    }

    pthread_exit(&perlin_thread);
    UnloadTexture(perlin_noise);
    UnloadTexture(edge_map);
    UnloadTexture(map);
    CloseWindow();
    list_free(&objects);
    free_map(&map_data);
    free_map(&map_edge_data);

    return 0;
}
