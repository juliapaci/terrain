#include "physics.h"

#include <stdlib.h>
#include <raylib.h>

// physics stuff
void draw_objects(Array *objs) {
    for(size_t i = 0; i < objs->used; i++) {
        const phys_Object obj = objs->list[i];
        DrawCircle(
                obj.x,
                obj.y,
                obj.radius,
                RED
            );
    }
}

// dynamic array
void array_init(Array *arr) {
    arr->list = malloc(ARR_SIZE * sizeof(phys_Object));
    arr->size = ARR_SIZE;
    arr-> used = 0;
}

void array_push(Array *arr, phys_Object obj) {
    if(arr->used == arr->size) {
        arr->size *= 2;
        arr->list = realloc(arr->list, arr->size * sizeof(phys_Object));
    }

    arr->list[arr->used++] = obj;
}

void array_clear(Array *arr) {
    free(arr->list); // should i do this?
    arr->list = malloc(ARR_SIZE * sizeof(phys_Object));
    arr->used = 0;
    arr->size = ARR_SIZE;
}

void array_free(Array *arr) {
    free(arr->list);
    arr->list = NULL;
    arr->used = 0;
    arr->size = 0;
}
