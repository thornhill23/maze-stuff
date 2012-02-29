#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>


#define N 4 // maze is size N x N

#define TURN90_TIME 9
#define TURN45_TIME 7 
#define ROT90_TIME 	10 // or something MUCH larger?
#define ROT180_TIME 17 // or something MUCH larger?
// i.e., should rotation cost be 'real' or 'prohibitive'?


#define HASH_SIZE (N * (N - 1) / 2) // 360 rough upper bound on # nodes used
#define HEAP_SIZE 258 // should be of form 2^k + 2 
					  // (since 1-indexed and need extra space at end)

#define VERT_TO_HORIZ(id) (id + N*(N-1)) // Vertical node to Horizontal node on same square
							  // (right to bottom)
#define HORIZ_TO_VERT(id) (id - N*(N-1)) // Horizontal to Vertical (bottom to right)

//	struct dist {
//		uint16_t id;
//		uint16_t time;
//	}

#ifndef FASTEST_PATH
#define FASTEST_PATH
struct node {
	uint16_t pre;
	uint16_t time;
	uint16_t id; // together, id and dir uniquely identify a node
	uint8_t dir; //		in the graph.
	/*		0
	 *	7		1 
	 * 6   		 2
	 *  5		3
	 *		4
	 */
	struct node *nxt_in_tbl; // 32 bits (?)
};

//struct nbhr {
// 	uint16_t id;
// 	uint16_t time;
// 	uint8_t dir;
//};

struct square {
	_Bool w[4];	 // walls ( 0 if wall, 1 if none. go figure.)
	/*  0
	 *3   1
	 *  2
	 */
};

struct map { 
	uint16_t start_id; 
	uint8_t start_dir; 
//	uint16_t current; // node at top of heap
	uint16_t end_id; 
	uint16_t n_heap; // current number of nodes in heap 
	uint16_t n_tbl;  // current number of nodes in hash table
	struct square *m; // maze 
};

void pop(struct node **heap, struct node **tbl, struct map *trip);
uint16_t time_straight(uint16_t n, struct node *p, struct node nbhrs[n-1]);
uint16_t just_ahead(uint16_t id, uint8_t dir);
struct node* turn90(uint16_t id, uint8_t dir);
_Bool node_at(uint16_t id);
void get_nbhrs(struct node *p, struct node *nbhrs, struct map *trip);

struct node* hash(struct node nb, uint16_t end_id, struct node **tbl);
void update_tbl(struct node *nbhrs, struct node **tbl);

void update_heap(struct node *nbhrs, struct node **heap, struct map *trip, \
				 struct node **tbl);
uint16_t parent(uint16_t c);
uint16_t child(uint16_t p);

//uint16_t* trace_back(uint16_t *path, struct node **tbl, struct map *trip);
uint16_t* fastest_path(struct map *trip);

#endif 
