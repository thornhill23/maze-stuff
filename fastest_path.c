#include "fastest_path.h"

// todo: 
// 2. figure out what the fastest path output looks like.
// 		(node movement node movement) or (node n ... n, movement m .. m) ?
// 3. wrt turning on a dime: different angles possible (45 90 180) (if at dead end) 
// 4. 'nbhrs' issue?:
// [0]			[1]		[2]		[3]
// orig_id		id1		id2		id3	
// num_id's		time1	time2	time3			

// 5. should we take start / end _squares_ instead of nodes?


uint16_t child(uint16_t p) {
	// add 1 for index of second child 
	return p << 1;
}

void pop(struct node **heap, struct node **tbl, struct map *trip) {
	heap[0] = heap[1];
	uint16_t p = 1; // parent
	uint16_t c = 2; // child
	uint16_t c_min; // child with lowest time
	while (c + 1 < trip->n_heap) {
		c_min = (heap[c]->time < heap[c + 1]->time) ? c : c + 1;
		heap[p] = heap[c_min];
		p = c_min;
		c = child(p);
	}

	heap[c_min] = heap[trip->n_heap--];
	return;
}
uint16_t time_straight(uint16_t n, struct node *p, struct node nb) {
	// may use p, nbhrs[n-1] to ask if diagonal and vary time cost 
	return (1  + (2 * n)); // const. penalty at beginning? nonlinear?
}

uint16_t just_ahead(uint16_t id, uint8_t dir) {
	// PROBLEM: setting uint16_t < 0. => "natural overflow"?
	if (id > 2*N*(N-1)) 
		return id;

	bool vertical = (id <= N*(N-1));

	switch (dir) {
		case 0: // "North" direction, node 'id' necessarily horizontal
			return (id - (N-1));
		case 4: // South, necessarily horizontal
			return (id + N-1);
		case 2: // E, necessarily vertical 
			return (id + N-1);
		case 6: // W, necessarily vertical 
			return (id - (N-1));
		case 1: // NE
			if (vertical)
				return (VERT_TO_HORIZ(id) + N-2);
			else
				return HORIZ_TO_VERT(id);
		case 3: // SE
			if (vertical)
				return (VERT_TO_HORIZ(id) + N-1);
			else
				return (HORIZ_TO_VERT(id) + N-1);
		case 5: // SW
			if (vertical)
				return VERT_TO_HORIZ(id); 
			else
				return (HORIZ_TO_VERT(id) + N-2);
		case 7: // NW
			if (vertical)
				return (VERT_TO_HORIZ(id) - (N-1)); 
			else
				return (HORIZ_TO_VERT(id) - 1);
		default: // returns an invalid node id
			return 0xffff; // 2^16 - 1
	}
}

struct node* turn90(uint16_t id, uint8_t dir) {
	struct node nb[3]; // nb[0] relative left
   					   // nb[1] relative right
	nb[0].dir = (dir + 6) % 8;
	nb[1].dir = (dir + 2) % 8;

	bool vertical = (id <= N*(N-1));

	switch (dir) {
		case 0: // "North" direction, node 'id' necessarily horizontal
			nb[0].id  = HORIZ_TO_VERT(id) - 1;
			nb[1].id  = HORIZ_TO_VERT(id);
			break;
		case 4: // South, necessarily horizontal
			nb[0].id  = HORIZ_TO_VERT(id+1);
			nb[1].id  = HORIZ_TO_VERT(id+1) - 1;
			break;
		case 2: // E, necessarily vertical 
			nb[0].id  = VERT_TO_HORIZ(id+1) - 1;
			nb[1].id  = VERT_TO_HORIZ(id+1);
			break;
		case 6: // W, necessarily vertical 
			nb[0].id  = VERT_TO_HORIZ(id);
			nb[1].id  = VERT_TO_HORIZ(id) - 1;
			break;
		case 1: // NE
			if (vertical)  {
				nb[0].id = id - (N-1);
				nb[1].id = id +1;
			} else {
				nb[0].id = id - 1;
				nb[1].id = id + N-1;
			}
			break;
		case 3: // SE
			if (vertical) {
				nb[0].id = id + 1;
				nb[1].id = id + N-1;
			} else {
				nb[0].id = id + N-1;
				nb[1].id = id + 1;
			}
			break;
		case 5: // SW
			if (vertical) {
				nb[0].id = id + 1;
				nb[1].id = id -(N-1);
			} else {
				nb[0].id = id + N-1;
				nb[1].id = id - 1;
			}
			break;
		case 7: // NW
			if (vertical) {
				nb[0].id = id - 1;
				nb[1].id = id - (N-1);
			} else {
				nb[0].id = id - (N-1);
				nb[1].id = id - 1;
			}
			break;
		default: // returns an invalid node id
			nb[0].id = 0xffff; // 2^16 - 1
			nb[1].id = 0xffff; // 2^16 - 1

	}
	return nb;
}


bool node_at(uint16_t id, struct square *m) {
	if (id > 2*N*(N-1))
		return false;

	uint8_t i,j; 
	if (id <= N * (N - 1)) { 
		// the potential node is on a vertical edge
		i = id / (N - 1);
		j = id % (N - 1);
		//  0 if wall (node_at = false), 1 if none (node_at = true)
		return m[ N * i  + j].w[1]; // right wall
	}
	// the potential node is on a horizontal edge
	j = id / (N - 1);
	i = id % (N - 1);
	return m[ N * i  + j].w[2]; // bottom wall
}

void get_nbhrs(struct node *p, struct node *nbhrs, struct map *trip) {
	nbhrs[0].id = p->id; // (0th elem).id stores "root" node
	uint8_t n = 0;

	// --====0---==--=-=-=--=====-------==--=------
	// if @ start node, rotation in place may be necessary
	if (p->id == trip->start_id) {
		nbhrs[++n].id = p->id; 
		nbhrs[n].dir = (p->dir + 4) % 8;
		nbhrs[n].time = p->time + ROT180_TIME; 
	}
	// haven't found use for 90deg rotation yet.
	// ? should we take start / end _squares_ instead of nodes?
	
	// straight ahead
	while(node_at(just_ahead(nbhrs[n].id,p->dir),trip->m)) {
		nbhrs[n+1].id = just_ahead(nbhrs[n].id,p->dir); 
		nbhrs[++n].dir = p->dir;
		nbhrs[n].time = p->time + time_straight(n, p, nbhrs[n-1]);
	}

	// --- 90deg turns ---
	struct node *nb;
	nb = turn90(p->id,p->dir); 
	for (int i = 0; i < 2; ++i) { // two possible 90deg turns given direction
		if (node_at(nb[i].id,trip->m)) {
			nbhrs[++n].id = nb[i].id;
			nbhrs[n].dir = nb[i].dir;
			nbhrs[n].time = p->time + TURN90_TIME;
		}
	}

	// --- 45deg turns --- 
	if (node_at(just_ahead(p->id,p->dir),trip->m)) {
		nb = turn90(just_ahead(p->id,p->dir),p->dir);
		if (p->id > N*(N-1) && node_at(nb[0].id,trip->m)) { 
			// starting node horizontal => only left 45 deg turn possible
			nbhrs[++n].id = nb[0].id;
			nbhrs[n].dir = (p->id + 7) % 8;
		} else if (node_at(nb[1].id,trip->m)) { 
			// starting node vertical => only right 45 deg turn possible
			nbhrs[++n].id = nb[1].id;
			nbhrs[n].dir = (p->id + 1) % 8;
		}
		nbhrs[n].time = p->time + TURN45_TIME;
	}

	nbhrs[0].time = n; // (0th elem).time stores # neighbors
	return;
}


struct node* hash(struct node *nb, uint16_t end_id, struct node **tbl) {
	struct node *p = tbl[nb->id];
	while (p->id != nb->id && (p->dir != nb->dir || p->id == end_id))
		p = p->nxt_in_tbl; 
	// safe if node is in table
	return p;
}

//uint16_t* trace_back(uint16_t *path, struct node **tbl, struct map *trip) {
//	struct node *nb;
//	nb->id = trip->end_id;
//	nb->dir = 0; // necessary?
//	path[0] = 0; // counts # nodes in path
//	do {
//		// modify nb (?) see todo at top
//		path[++path[0]] = nb.id;
//		nb = hash(nb,trip->end_id,tbl)->pre;
//	} while (nb.id != trip->start_id);
//
//	path[++path[0]] = trip->start_id;
//	free(tbl);
//	return path;
//	// SOMEONE NEEDS TO FREE 'path'!
//}

void update_tbl(struct node *nbhrs, struct node **tbl) {
	uint16_t k;
	struct node *c = malloc(sizeof(struct node));
	for (uint8_t i = 1; i <= nbhrs[0].time; ++i) { 
		k = nbhrs[i].id % HASH_SIZE;
		c = tbl[k];
		// problem: seg fault 
		while (!(c->id == nbhrs[i].id && c->dir == nbhrs[i].dir)) {
			if (c == NULL) { // initialize node
				c = malloc(sizeof(struct node*));
				c->id  = nbhrs[i].id; 
				c->dir  = nbhrs[i].dir; 
				c->time = nbhrs[i].time; 
				c->nxt_in_tbl = NULL;
				break;
			}  
			c = c->nxt_in_tbl; 
		}
		if (nbhrs[i].time < c->time) {
			c->time = nbhrs[i].time;
			c->pre_id = nbhrs[0].id; // (0th elem).id stores current node
			c->pre_dir = nbhrs[0].dir; // (0th elem).id stores current node
		}
	}
	free(c);
	return;
}

uint16_t parent(uint16_t c) {
	return ((c == 1) ? 1 : (c >> 1));
}

void update_heap(struct node *nbhrs, struct node **heap, struct map *trip, \
				 struct node **tbl) {
	uint16_t nc, np; 
	struct node *temp;
	for (int8_t i = 1; i <= nbhrs[0].id; ++i) { 
		nc = ++trip->n_heap;
		heap[nc] = hash(&nbhrs[i],trip->end_id,tbl); // heap(nc), heap(np) ; not nc, np in the following ??
		np = parent(nc);
		while (heap[np]->time > heap[nc]->time) {
			// switch node ID (hash key) positions in the heap.
			temp = heap[np]; 
			heap[np] = heap[nc];
			heap[nc] = temp;
			nc = np;
			np = parent(nc);
		} 
		
		if (trip->n_heap == HEAP_SIZE) 
			--trip->n_heap;
	}
	return;
}


struct node** fastest_path(struct map *trip) {
	trip->n_tbl = 0;
	trip->n_heap = 0;
//	struct node *heap[2 + HEAP_SIZE]; // 1-indexed + need temporary space at end
	struct node **heap = malloc( HEAP_SIZE * sizeof(struct node *)); 
	struct node **tbl = malloc( HASH_SIZE * sizeof(struct node *)); 
	for (uint16_t i = 0; i < HASH_SIZE; ++i)
		tbl[i] = NULL;

	// 7 + N is approx max possible number of connected nodes (neighbors)
	struct node *nbhrs = malloc( (7 + N) * sizeof(struct node *)); 
	// problem spot
	nbhrs[0].time = 1;
	nbhrs[0].id = 0xffff; // (not a valid node, since start has no previous node)
	nbhrs[1].id = trip->start_id;
	nbhrs[1].dir = trip->start_dir;
	nbhrs[1].time = 0;
	update_tbl(nbhrs,tbl);
	heap[1] = hash(&nbhrs[1],trip->end_id,tbl);

	do {
		pop(heap,tbl,trip);
		get_nbhrs(heap[0],nbhrs,trip);
		update_tbl(nbhrs,tbl);
		update_heap(nbhrs,heap,trip,tbl);
	} while (heap[0]->id != trip->end_id); 

	free(nbhrs);
	free(heap);
//	return trace_back(path,tbl,trip);
	return tbl;
	// someone needs to free this at the receiving end.
}
