#include <stdlib.h>
#include <stdio.h>
#include <raylib.h>

#include "perlin.h"

#define DATA_SIZE 100
#define OCTAVE_AMOUNT 12
#define WIDTH 1920
#define HEIGHT 1080

unsigned char *get_perlin() {
    unsigned char *perlin_data = malloc(WIDTH*HEIGHT*4);
    if(!perlin_data)
        return NULL;
    for(int x = 0; x < WIDTH; x++) {
        for(int y = 0; y < HEIGHT; y++) {
            int index = (y * WIDTH + x) * 4;

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
            perlin_data[index] = colour;        // R
            perlin_data[index + 1] = colour;    // G
            perlin_data[index + 2] = colour;    // B
            perlin_data[index + 3] = (unsigned char) 256; // A
        }
    }

    return perlin_data;
}

int main(void) {
        SetConfigFlags(FLAG_WINDOW_RESIZABLE);
        InitWindow(0, 0, "Terrain");
        unsigned char *perlin_data = get_perlin();
        if(perlin_data == NULL)
            CloseWindow();

        // Image perlin_noise_img = LoadImageFromMemory(PIXELFORMAT_UNCOMPRESSED_R8G8B8, perlin_data, WIDTH*HEIGHT*4);
        Image perlin_noise_img = {
            perlin_data,
            WIDTH,
            HEIGHT,
            PIXELFORMAT_UNCOMPRESSED_GRAYSCALE,
            1
        };
        printf("%d", perlin_data[011]);
        Texture2D perlin_noise = LoadTextureFromImage(perlin_noise_img);

        while (!WindowShouldClose()) {
            BeginDrawing();
            ClearBackground(RAYWHITE);
            DrawTexture(perlin_noise, 0, 0, WHITE);
            EndDrawing();
        }

        UnloadTexture(perlin_noise);
        CloseWindow();
        free(perlin_data);

        return 0;
}
