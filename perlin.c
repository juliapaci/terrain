#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <pthread.h>

#include "perlin.h"

float perlin(float x, float y) {
    int x0 = (int)x;
    int y0 = (int)y;
    int x1 = x0 + 1;
    int y1 = y0 + 1;


    float wx = x - x0;
    float wy = y - y0;

    float high = interpolate(
            dot_off_grad(x0, y0, x, y),
            dot_off_grad(x1, y0, x, y),
            wx
        );

    float low = interpolate(
            dot_off_grad(x0, y1, x, y),
            dot_off_grad(x1, y1, x, y),
            wx
    );

    return interpolate(
            high,
            low,
            wy
        );
}

float dot_off_grad(int x, int y, float px, float py) {
    perlin_Vector2 grad = gradient_hash(x, y);
    perlin_Vector2 dist = {
        x - px,
        y - py
    };

    return grad.i*dist.i + grad.j*dist.j;
}

// for hashing implementation:
// https://en.wikipedia.org/wiki/Perlin_noise#Implementation
perlin_Vector2 gradient_hash(int x, int y) {
    const unsigned int w = 8 * sizeof(unsigned);
    const unsigned int s = w / 2; // rotation width
    unsigned int a = x, b = y;
    a *= 3284157443; b ^= a << s | a >> (w-s);
    b *= 1911520717; a ^= b << s | b >> (w-s);
    a *= 2048419325;
    float random = a * (3.14159265 / ~(~0u >> 1));
    perlin_Vector2 v = {
         cos(random),
         sin(random)
    };
    return v;
}

float interpolate(float a, float b, float w) {
    return (b - a) * ((w * (w * 6.0 - 15.0) + 10.0) * w * w * w) + a;
}

void *get_perlin(void *arguments) {
    get_perlin_args *args = (get_perlin_args *)arguments;
    unsigned char *perlin_data = malloc(WIDTH*HEIGHT);
    if(!perlin_data)
        return NULL;
    for(int x = args->x; x < WIDTH+args->x; x++) {
        for(int y = args->y; y < HEIGHT+args->y; y++) {
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
            perlin_data[(y-(int)args->y) * WIDTH + (x-(int)args->x)] = colour;
        }
    }

    // TODO: use pthread exit or simply return?
    pthread_exit(perlin_data);
    // return perlin_data;
}
