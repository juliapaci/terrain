#include "physics.h"

#include <stdlib.h>
#include <raylib.h>
#include <math.h>

// physics stuff
void draw_objects(List *objs) {
    Node *obj = objs->head;
    while(obj != NULL) {
        DrawCircle(
                obj->value.x,
                obj->value.y,
                obj->value.radius,
                RED
            );
        obj = obj->next;
    }
}

// TODO: proper physics
void apply_physics(List *objs) {
    const int padding = -10;
    Node *obj = objs->head;
    while(list_size(objs) > LIST_CAP)
        list_dequeue(objs);
    while(obj != NULL) {
        phys_Object *val = &obj->value;
        if(obj->value.y > GetScreenHeight() + padding) {
            Node *tmp = obj->next;
            list_delete(objs, obj);
            obj = tmp;
            continue;
        }

        val->vec.j += GRAVITY;
        val->vec.i -= FRICTION;

        // cap velocity
        if(val->vec.mag > TERM_VELO)
            val->vec.mag = TERM_VELO;

        val->x += (val->vec.i*val->vec.mag)/256 * FRICTION * 1/val->mass;
        val->y += (val->vec.j*val->vec.mag)/256 * FRICTION * 1/val->mass;
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

void list_dequeue(List *list) {
    if(list->head == NULL || list->tail == NULL)
        return;
    Node *tmp = list->head;
    list->head = list->head->next;
    free(tmp);
}

void list_delete(List *list, Node *target) {
    if(target == NULL)
        return;
    if(target->prev != NULL)
        target->prev->next = target->next;
    if(target->next != NULL)
        target->next->prev = target->prev;
    else
        list->head = target->next;
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
