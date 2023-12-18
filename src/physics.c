#include "physics.h"

#include <stdlib.h>
#include <raylib.h>
#include <math.h>

// physics stuff
void draw_objects(List *objs, bool show_objects) {
    Node *obj = objs->head;
    while(obj != NULL) {
        phys_Object *val = &obj->value;

        DrawCircle(
                val->x,
                val->y,
                val->radius,
                RED
            );

        if(show_objects) {
            // x dir
            DrawLine(
                val->x,
                val->y,
                val->x + val->vec.i,
                val->y,
                GREEN
            );
            // y dir
            DrawLine(
                val->x,
                val->y,
                val->x,
                val->y + val->vec.j,
                GREEN
            );
            // dir
            DrawLine(
                val->x,
                val->y,
                val->x + val->vec.i,
                val->y + val->vec.j,
                BLUE
            );
        }

        obj = obj->next;
    }
}

// TODO: proper physics
void apply_physics(List *objs) {
    const int padding = 10;
    Node *obj = objs->head;

    while(obj != NULL) {
        phys_Object *val = &obj->value;
        if(val->y > GetScreenHeight() - padding*-val->mass) {
            Node *tmp = obj->next;
            list_delete(objs, obj);
            obj = tmp;
            continue;
        }

        val->vec.j += GRAVITY*val->mass;
        val->vec.i -= FRICTION*val->mass;

        // cap velocity
        if(val->vec.mag > TERM_VELO)
            val->vec.mag = TERM_VELO;

        val->x += (val->vec.i*val->vec.mag)/256 * FRICTION * 1/val->mass;
        val->y += (val->vec.j*val->vec.mag)/256 * FRICTION * 1/val->mass;
        obj = obj->next;
    }
}

// TODO: use quadtree for this (see if its slower)
void collide_environment(phys_Object *obj, bool **map) {
    const int min_x = obj->x - obj->radius;
    const int max_x = obj->x + obj->radius;
    const int min_y = obj->y - obj->radius;
    const int max_y = obj->y + obj->radius;

    if(min_x < 0 || max_x > GetScreenWidth() || min_y < 0 || max_y > GetScreenHeight())
        return;

    bool hit_vert = false;
    bool hit_hori = false;
    for(int x = min_x; x < max_x; x++) {
        for(int y = min_y; y < max_y; y++) {
            if(!map[x][y])
                continue;

            if(y > min_y || y < max_y)
                hit_vert = true;
            if(x > min_x || x < max_x)
                hit_hori = true;
        }
    }

    #define OPP_DIR(D) (obj->vec.D = -(obj->vec.D))

    if(hit_vert)
        OPP_DIR(j);
    if(hit_hori)
        OPP_DIR(i);
}

void collide_physics(List *objs, bool **map_data, bool environment) {
    Node *obj = objs->head;

    while(obj != NULL) {
        if(environment)
            collide_environment(&obj->value, map_data);

        // collide_objs();

        obj = obj->next;
    }
}

// list
void list_enqueue(List *list, phys_Object obj) {
    Node *node = malloc(sizeof(Node));
    node->prev = NULL;
    node->value = obj;
    node->next = NULL;

    if(list->head == NULL || list->tail == NULL) {
        list->head = node;
        list->tail = node;
        return;
    }

    list->tail->next = node;
    node->prev = list->tail;
    list->tail = node;
}

// TODO: sometimes crashes here
void list_dequeue(List *list) {
    if(list->head == NULL || list->tail == NULL)
        return;
    Node *tmp = list->head;
    list->head = list->head->next;
    free(tmp);
}

// TODO: bug where if you delete the most recently created object while there is still existing objects in the list, it wont create the new object.
void list_delete(List *list, Node *target) {
    if(target == NULL)
        return;
    if(list->head == target)
        list->head = target->next;
    if(target->prev != NULL)
        target->prev->next = target->next;
    if(target->next != NULL)
        target->next->prev = target->prev;
    free(target);
}

// TODO: maybe there is a mem leak here
void list_free(List *list) {
    Node *node = list->head;
    while(node != NULL) {
        Node *tmp = node;
        node = node->next;
        free(tmp);
        tmp = NULL;
    }
    list->head = NULL;
    list->tail = NULL;
}

size_t list_size(List *list) {
    int size;
    Node *node = list->head;
    for(size = 0; node != NULL; size++)
        node = node->next;
    return size;
}
