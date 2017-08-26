#ifndef __AI__
#define __AI__

#include <stdint.h>
#include <unistd.h>
#include "node.h"
#include "priority_queue.h"


void initialize_ai(int max_depth);
void deinitialize_ai(void);

move_t get_next_move( uint8_t board[SIZE][SIZE], int max_depth, propagation_t propagation, stats_t *statistics );

#endif
