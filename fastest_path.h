#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <math.h>

#ifndef FASTEST_PATH
#define FASTEST_PATH

#define N 4 // maze is size N x N

//#define TURN90_TIME 9
//#define TURN45_TIME 7 
//#define ROT90_TIME 	10 // or something MUCH larger?
//#define ROT180_TIME 17 // or something MUCH larger?
// i.e., should rotation cost be 'real' or 'prohibitive'?

#define HASH_SIZE (N * (N - 1) / 2) // 360 rough upper bound on # nodes used
#define HEAP_SIZE 258 // should be of form 2^k + 2 
					  // (since 1-indexed and need extra space at end)
#define MAX_NBHRS (4 * N)

#define VERT_TO_HORIZ(id) (id + N*(N-1)) // Vertical node to Horizontal node on same square
							  // (right to bottom)
#define HORIZ_TO_VERT(id) (id - N*(N-1)) // Horizontal to Vertical (bottom to right)
	/*		0
	 *	7		1 
	 * 6   		 2
	 *  5		3
	 *		4
	 */
struct node {
	uint16_t time;
	uint16_t id; // together, id and dir uniquely identify a node
	uint16_t pre;
	struct node *nxt_in_tbl; // 32 bits (?)
};

struct heap {
	struct node *h[HEAP_SIZE]; // or *node h[]?
	uint16_t n_heap; // current number of nodes in heap 
};

struct tbl {
	struct node *t[HASH_SIZE];
	uint16_t n_tbl;  // current number of nodes in hash table
};

struct nbhs {
	struct node n[MAX_NBHRS];
	uint16_t n_nbhs;  // current number of nodes in hash table
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

struct maze { 
	uint16_t start_id; 
	uint16_t end_id; 
	struct square *m; // maze 
};

void pop(struct heap *heap, struct tbl *tbl, struct maze *maze);
uint16_t time_straight(uint16_t n, struct node *p, struct node nb);
uint16_t just_ahead(uint16_t id, uint8_t dir);
bool node_at(uint16_t id, struct square *m);
void get_nbhs(struct node *p, struct nbhs *nbhs, struct maze *maze);

struct node* hash(struct node* nb, struct tbl *tbl);
void update_tbl(struct nbhs *nbhs, struct tbl *tbl);

void update_heap(struct nbhs *nbhs, struct heap *heap, struct maze *maze, \
				 struct tbl *tbl);
uint16_t parent(uint16_t c);
uint16_t child(uint16_t p);
//uint16_t* trace_back(uint16_t *path, struct tbl tbl, struct maze *maze);
struct tbl* fastest_path(struct maze *maze);

#endif 
