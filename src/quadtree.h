#ifndef __QUADTREE_H__
#define __QUADTREE_H__

#include <raylib.h>
#include "physics.h"

// TODO: fix capactiy only working for value 1
#define CAPACITY 1 // amount of objects in a quad for a sub division to occur
#define MAX_DEPTH 10 // max child nodes

typedef struct QuadTreeNode {
    struct QuadTreeNode *children; // represent node quadrants
    int x, y;
    int width, height;
    size_t amount;
    size_t depth;
} QuadTreeNode;

void init_tree(QuadTreeNode *node);
void generate_tree(QuadTreeNode *node, List *objs);
void subdivide(QuadTreeNode *node);
bool contains(QuadTreeNode *node, phys_Object *obj);

void query_tree(QuadTreeNode *node, Rectangle zone);
bool contains_rec(QuadTreeNode *node, Rectangle zone);

void draw_tree(QuadTreeNode *node);

#endif // __QUADTREE_H__
