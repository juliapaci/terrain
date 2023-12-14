#ifndef __PERLIN_H__
#define __PERLIN_H__

typedef struct {
    float i;
    float j;
} perlin_Vector2;

typedef struct {
    float x;
    float y;
} get_perlin_args;

#define DATA_SIZE 500
#define OCTAVE_AMOUNT 3
#define WIDTH 1920
#define HEIGHT 1080

void *get_perlin(void *arguments);                          // sample perlin noise into an array (freed by caller)
float perlin(float x, float y);                             // sample noise
float dot_off_grad(int x, int y, float px, float py);       // computes the dot of the distance ('offset') and gradient vector
perlin_Vector2 gradient_hash(int x, int y);                 // compute random gradient vector
float interpolate(float a, float b, float w);               // interpolate with weight

#endif // __PERLIN_H__
