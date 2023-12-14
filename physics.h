#ifndef __PHYSICS_H__
#define __PHYSICS_H__

#include <stddef.h>
#include <raylib.h>

#define LIST_CAP 20
#define TERM_VELO 10
#define GRAVITY 9.81
#define FRICTION 0.9

typedef struct {
    int i;
    int j;
    int mag; // save in mem so instead of calculating sqrt twice now we only have to do it once (each fram)
} phys_Vector;

typedef struct {
    phys_Vector vec;
    int mass;
    int radius;
    int x;
    int y;
} phys_Object;

// list
typedef struct Node {
    struct Node *prev; // only needs to be doubly linked for deletion
    phys_Object value;
    struct Node *next;
} Node;

typedef struct {
    Node *head;
    Node *tail;
} List;

void list_enqueue(List *list, phys_Object obj);
void list_dequeue(List *list);
void list_delete(List *list, Node *target);
void list_free(List *list);
size_t list_size(List *list);

// physics stuff
void draw_objects(List *objs, bool show_objects, Camera2D camera);
void apply_physics(List *objs);
void collide_physics(List *objs, bool **edge_data, bool environment);

#endif // __PHYSICS_H__
