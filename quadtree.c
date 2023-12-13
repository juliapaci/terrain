#include <raylib.h>
#include <stddef.h>
#include <stdlib.h>

#include "physics.h"
#include "quadtree.h"

#define GEN_QTNODE(X, Y, D) ((QuadTreeNode) {   \
            NULL,                               \
            node->x + node->width*X,            \
            node->y + node->height*Y,           \
            node->width/2,                      \
            node->height/2,                     \
            0,                                  \
            D                                   \
        })

void init_tree(QuadTreeNode *node) {
    // initial root node
    *node = (QuadTreeNode) {
        NULL,
        0, 0,
        GetScreenWidth(), GetScreenHeight(),
        0,
        0
    };
}

void generate_tree(QuadTreeNode *node, List *objs) {
    // TODO: how can we only regenerate changed part of the tree instead of entire tree?
    Node *list_node = objs->head;
    while(list_node != NULL) {
        if(!contains(node, &list_node->value)) {
            list_node = list_node->next;
            continue;
        }

        if(node->amount < CAPACITY-1) {
            node->amount++;
        } else {
            if(node->depth > MAX_DEPTH)
                return;
            if(node->children == NULL)
                subdivide(node);

            generate_tree(&node->children[0], objs);
            generate_tree(&node->children[1], objs);
            generate_tree(&node->children[2], objs);
            generate_tree(&node->children[3], objs);
        }

        list_node = list_node->next;
    }
}

void subdivide(QuadTreeNode *node) {
    if(node->children != NULL)
        return;

    node->children = malloc(sizeof(QuadTreeNode) * 4);
    const size_t depth = node->depth+1;

    node->children[0] = GEN_QTNODE(1/2, 0, depth);
    node->children[1] = GEN_QTNODE(0, 0, depth);
    node->children[2] = GEN_QTNODE(0, 1/2, depth);
    node->children[3] = GEN_QTNODE(1/2, 1/2, depth);
}

bool contains(QuadTreeNode *node, phys_Object *obj) {
    return node->x < obj->x - obj->radius &&
        node->x + node->width > obj->x + obj->radius &&
        node->y < obj->y - obj->radius &&
        node->y + node->height > obj->y + obj->radius;
}


void draw_tree(QuadTreeNode *tree) {
    DrawRectangleLines(tree->x, tree->y, tree->width, tree->height, PURPLE);

    if(tree->children == NULL)
        return;
    draw_tree(&tree->children[0]);
    draw_tree(&tree->children[1]);
    draw_tree(&tree->children[2]);
    draw_tree(&tree->children[3]);
}
