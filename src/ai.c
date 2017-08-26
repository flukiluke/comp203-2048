/*******************************
  Assignment 2 - 2048 Solver
  Luke Ceddia

Attempts to play the game 2048.
 It even wins sometimes, too!
********************************/

#include <time.h>
#include <stdlib.h>
#include <assert.h>
#include "ai.h"
#include "utils.h"
#include "priority_queue.h"

/* visited does not strictly need to be a priority queue, but it is convenient
   to do so */
struct heap to_visit;
struct heap visited;

/* Local functions */
node_t *create_node(uint8_t board[SIZE][SIZE]);
void propagate_score_max(node_t *node);
void propagate_score_avg(node_t *node);
move_t select_best(node_t **choices);
int count_tiles(uint8_t board[SIZE][SIZE]);
int edgeness(uint8_t board[SIZE][SIZE]);
int rate_node(node_t *node);


void initialize_ai(int max_depth){
  heap_init(&to_visit);
  heap_init(&visited);
}

void deinitialize_ai(void) {
  heap_free(&to_visit);
  heap_free(&visited);
}

node_t *create_node(uint8_t board[SIZE][SIZE]) {
  node_t *node;
  node = malloc(sizeof(node_t));
  assert(node);
  node->num_children = 0;
  node->priority = 0;
  memcpy(node->board, board, SIZE * SIZE);
  return node;
}

/* These two functions are alternate methods for propagating a single node's
   score to the first-level move node */
void propagate_score_max(node_t *node) {
  int max_score = 0;
  while (node->depth > 0) {
    max_score = node->priority > max_score ? node->priority : max_score;
    node = node->parent;
  }
  node->priority = max_score;
}

void propagate_score_avg(node_t *node) {
  node_t *parent;
  parent = node;
  while (parent->depth > 1) parent = parent->parent;
  parent->num_children++;
  parent->priority = (parent->priority * (parent->num_children - 1) + node->priority) / parent->num_children;
}


/* Given an array of nodes, pick the one with the highest score.
   Randomly selects if there is a tie. */
move_t select_best(node_t **choices) {
  int best_score = -1;
  move_t best_move = left;
  int i;
  for (i = 0; i <= 3; i++) {
    if (choices[i]) {
      if (choices[i]->priority > best_score) {
	best_score = choices[i]->priority;
	best_move = choices[i]->move;
      }
      else if (choices[i]->priority == best_score) {
	if (rand() > RAND_MAX / 2) {
	  best_score = choices[i]->priority;
	  best_move = choices[i]->move;
	}
      }
    }
  }
  return best_move;  
}

/* These functions below are for helping rate a board state */

/* Simply count the number of tiles on the board */
int count_tiles(uint8_t board[SIZE][SIZE]) {
  int i, j;
  int count = 0;
  for (i = 0; i < SIZE; i++) {
    for (j = 0; j < SIZE; j++) {
      if (board[i][j]) count++;
    }
  }
  return count;
}

/* Give a representation of how many tiles are on the edge */
int edgeness(uint8_t board[SIZE][SIZE]) {
  int i, j;
  int score = 0;
  for (i = 0; i < SIZE; i++) {
    for (j = 0; j < SIZE; j++) {
      if (i == 0 || i == SIZE || j == 0 || j == SIZE) {
        score += board[i][j];
      }
    }
  }
  return score;
}

/* Give an overall score for how good a board state is */
int rate_node(node_t *node) {
  int tile_count = count_tiles(node->board);
  /* Values below chosen such that a full board gives a score of 0 */
  return (160 - 10 * tile_count) * edgeness(node->board);
}

/**
 * Find best action by building all possible paths up to depth max_depth
 * and back propagate using either max or avg
 */

move_t get_next_move(uint8_t board[SIZE][SIZE], int max_depth, propagation_t propagation, stats_t *statistics) {
    node_t *node, *newnode;
    int move, changed;
    clock_t start_time = clock();
    uint32_t score;
    node_t *next_moves[4]; /* Stores top-level moves */
    move_t best_move;

    /* A depth of 0 means that we do not search any possibilities */
    if (max_depth == 0) return rand() % 4;

    node = create_node(board);
    node->parent = NULL;
    node->depth = 0;
    heap_push(&to_visit, node);

    while (to_visit.count) {
      node = heap_delete(&to_visit);     
      statistics->expanded++;
      heap_push(&visited, node);
      /* Don't process nodes past a certain depth */
      if (node->depth < max_depth) {
	for (move = 0; move <= 3; move++) {
          newnode = create_node(node->board);
          score = 0;
          changed = execute_move_t(newnode->board, &score, move);
          statistics->generated++;
          /* Only further process nodes that are valid moves */
          if (changed) {
            newnode->parent = node;
            newnode->depth = node->depth + 1;
            newnode->move = move;
            newnode->priority = rate_node(newnode);
            heap_push(&to_visit, newnode);

            if (propagation == max) {
              propagate_score_max(newnode);
            }
            else {
              propagate_score_avg(newnode);
            }
            addRandom(newnode->board);
          }
          else {
            free(newnode);
          }
          /* Is newnode one of the four top-level moves?
             Conditional on changed avoids inserted a free'd node */
          if (node->depth == 0) next_moves[move] = changed ? newnode : NULL;
	}
      }
    }
    /* Select best move and free memory */
    best_move = select_best(next_moves);
    emptyPQ(&visited);
    statistics->total_time += (double)(clock() - start_time) / CLOCKS_PER_SEC;
    return best_move;
}
