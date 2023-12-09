#include <stdio.h>
#include <math.h>

#include "perlin.h"

float perlin(float x, float y) {
    int x0 = (int)x;
    int y0 = (int)y;
    int x1 = x0 + 1;
    int y1 = y0 + 1;


    float wx = x - x0;
    float wy = y - y0;

    float left = interpolate(
            dot_off_grad(x0, y0, x, y),
            dot_off_grad(x1, y0, x, y),
            wx
        );

    float right = interpolate(
            dot_off_grad(x0, y1, x, y),
            dot_off_grad(x1, y1, x, y),
            wx
    );

    return interpolate(
            left,
            right,
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
