#include "fastest_path.h"

// todo: 
// 2. figure out what the fastest path output looks like.
// 		(node movement node movement) or (node n ... n, movement m .. m) ?
// 3. wrt turning on a dime: different angles possible (45 90 180) (if at dead end) 
// 4. 'nbhs' issue?:
// [0]			[1]		[2]		[3]
// orig_id		id1		id2		id3	
// num_id's		time1	time2	time3			

// 5. should we take start / end _squares_ instead of nodes?

uint16_t child(uint16_t p) {
	// add 1 for index of second child 
	return p << 1;
}

void pop(struct heap *heap, struct tbl *tbl, struct maze *maze) {
	heap->h[0] = heap->h[1]; // h[0] is "popped" 
	uint16_t p = 1; // parent
	uint16_t c = 2; // child
	uint16_t c_min = 1; // child with lowest time
	while (c + 1 <= heap->n_heap) {
		c_min = ((heap->h[c])->time < (heap->h[c + 1])->time) ? c : c + 1;
		heap->h[p] = heap->h[c_min];
		p = c_min;
		c = child(p);
	}

	heap->h[c_min] = heap->h[heap->n_heap--];
	return;
}
uint16_t time_straight(uint16_t n, struct node *p, struct node nb) {
	// may use p, nbhs->n[n-1] to ask if diagonal and vary time cost 
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

bool node_at(uint16_t id, struct square *m) {
	if (id > 2*N*(N-1))
		return false;

	uint8_t i,j; 
	if (id <= N*(N-1)) { 
		// the potential node is on a vertical edge
		i = id / N; 
		j = id % N;
		//  0 if wall (node_at = false), 1 if none (node_at = true)
		return m[ N * i  + j].w[1]; // right wall
	}
	// the potential node is on a horizontal edge
	j = id / N;
	i = id % N;
	return m[ N * i  + j].w[2]; // bottom wall
}

void get_nbhs(struct node *p, struct nbhs *nbhs, struct maze *maze) {
	// p is the popped node
	printf("get_nbhrs: popped id =%u\n",p->id);
	nbhs->n[0].id = p->id; // 0th elem stores "root" node
	nbhs->n[0].time = p->time; 
	nbhs->n_nbhs = 0; // n_nbhs tracks number of neighbors stored
	uint16_t k = 0;

	for (int dir = 0; dir < 8; ++dir) {
		while(node_at(just_ahead(nbhs->n[k].id,dir),maze->m)) {
			nbhs->n[k+1].id = just_ahead(nbhs->n[k].id,dir); 
			nbhs->n[k+1].time = p->time + time_straight(k, p, nbhs->n[k]);
			++k;
		}
		nbhs->n_nbhs += k;
		k = 0;
	}
	return;
}


struct node* hash(struct node *nd, struct tbl *tbl) {
	struct node *p;
	printf("hash: nd->id=%u\n",nd->id % HASH_SIZE);
    p = tbl->t[nd->id % HASH_SIZE];
	printf("nxt_in_tbl=NULL? %d\n",(p->nxt_in_tbl==NULL) ? 1:0);
	while (p != NULL) {
		if (p->id == nd->id) break;
		else p = p->nxt_in_tbl;
	}
	// if node not in table, returns NULL
	if (p != NULL)
		printf("hash: returned p->id= %u\n",p->id);
	else 
		printf("hash: returning NULL\n");
	return p;
}

void update_tbl(struct nbhs *nbhs, struct tbl *tbl) {
	uint16_t k; // hash table index
	struct node *c;
	bool added = false;
	for (uint8_t i = 1; i <= nbhs->n_nbhs; ++i) { 
		k = nbhs->n[i].id % HASH_SIZE;
		printf("update_tbl: index = %u\n",k);
		c = tbl->t[k];
		if (c == NULL) {
			c = malloc(sizeof(struct node));
			c->id = nbhs->n[i].id;
			c->time = nbhs->n[i].time;
			c->pre = nbhs->n[0].id;
			c->nxt_in_tbl = NULL;
			tbl->t[k] = c;
		}

		while (c != NULL) {
			if (c->id == nbhs->n[i].id) {
				if (nbhs->n[i].time < c->time) {
					c->time = nbhs->n[i].time;
					c->pre = nbhs->n[0].id;
				}
				added = true;
				break;
			}
			if (c->nxt_in_tbl == NULL) break;
			else c = c->nxt_in_tbl;
		}

		if (!added && c->nxt_in_tbl == NULL) {
			c->nxt_in_tbl = malloc(sizeof(struct node));
			c = c->nxt_in_tbl;
			c->id = nbhs->n[i].id;
			c->time = nbhs->n[i].time;
			c->pre = nbhs->n[0].id;
			c->nxt_in_tbl = NULL;
		}
	}
	return;
}

uint16_t parent(uint16_t c) {
	return ((c == 1) ? 1 : (c >> 1));
}

void update_heap(struct nbhs *nbhs, struct heap *heap, struct maze *maze, \
				 struct tbl *tbl) {
	uint16_t nc, np; 
	struct node *temp;
	for (int8_t i = 1; i <= nbhs->n_nbhs; ++i) { 
		nc = ++heap->n_heap;
		heap->h[nc] = hash(&(nbhs->n[i]),tbl);
		if (heap->h[nc] != NULL)
			printf("hash: returned p->id= %u\n",heap->h[nc]->id);
		else 
			printf("hash: returning NULL\n");
		np = parent(nc);
		while (heap->h[np]->time > heap->h[nc]->time) {
			// switch node ID (hash key) positions in the heap
			temp = heap->h[np]; 
			heap->h[np] = heap->h[nc];
			heap->h[nc] = temp;
			nc = np;
			np = parent(nc);
		} 
		
		if (heap->n_heap == HEAP_SIZE) 
			--heap->n_heap;
	}
	return;
}

void fastest_path(struct maze *maze, struct tbl *tbl) {
	tbl->n_tbl = 0;
	for (uint16_t i = 0; i < HASH_SIZE; ++i)
		tbl->t[i] = NULL;

	struct heap *heap = malloc(sizeof(struct heap)); 

	struct nbhs *nbhs = malloc(sizeof(struct nbhs)); 
	nbhs->n_nbhs = 1;
	nbhs->n[0].id = 0xffff; // (invalid node, since start has no previous node)
	nbhs->n[0].time = 87; //irrelevant, won't be used
	nbhs->n[1].id = maze->start_id;
	nbhs->n[1].time = 0;

	update_tbl(nbhs,tbl);
	heap->h[1] = hash(&(nbhs->n[1]),tbl);
	heap->n_heap = 1;

	do {
		pop(heap,tbl,maze);
		get_nbhs(heap->h[0],nbhs,maze);
		printf("nnb=%u\n",nbhs->n_nbhs);
		update_tbl(nbhs,tbl);
		update_heap(nbhs,heap,maze,tbl);
		printf("nhp=%u\n",heap->n_heap);
	} while (heap->h[0]->id != maze->end_id); 

	free(heap);
	free(nbhs);
	return;
}
