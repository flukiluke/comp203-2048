#ifndef __NODE__
#define __NODE__

#include "utils.h"

/**
 * Data structure containing the node information
 */
struct node_s{
    int priority;
    int depth;
    int num_children;
    move_t move;
    uint8_t board[SIZE][SIZE];
    struct node_s* parent;
};

typedef struct node_s node_t;


#endif
