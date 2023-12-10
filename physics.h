#ifndef __PHYSICS_H__
#define __PHYSICS_H__

#include <stddef.h>

#define ARR_SIZE 10
#define TERM_VELO 100
#define GRAVITY 0.981

typedef struct {
    int i;
    int j;
    int mag; // save in mem so instead of calculating sqrt twice now we only have to do it once (each fram)
} phys_Vector;

typedef struct {
    phys_Vector vec;
    int radius;
    int x;
    int y;
} phys_Object;

// dynamic array
typedef struct {
    size_t size;
    size_t used;
    phys_Object *list;
} Array;

void array_init(Array *arr);
void array_push(Array *arr, phys_Object obj);
void array_clear(Array *arr);
void array_free(Array *arr);

// physics stuff
void draw_objects(Array *objs);
void apply_physics(Array *objs);


#endif // __PHYSICS_H__
