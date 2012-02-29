#include <ncurses.h> // apparently -lncurses is necessary here as a gcc option
#include "fastest_path.h"

struct submaze { // could use 4-bitfields to save space
	struct square *m; /* pointer to original maze */

	/* row, column offsets from original pointer */
	uint8_t i; 
	uint8_t j; 

	/*number of empty rows, cols in submaze avail. for wall placement */
	uint8_t r;
	uint8_t c;
};

//void draw(struct square *m) 
//{
//	for (int i = 0; i < N; ++i) {
//		/* top/bottom walls */
//		for (int j = 0; j < N; ++j) {	
//			printf("+");
//			if (m[i * N + j].w[0])
//				printf("--");
//			else 
//				printf("  ");
//		}
//		printf("+");
//		printf("\n");
//
//		/* left/right walls */
//		for (int j = 0; j < N; ++j) {	
//			if (m[i * N + j].w[3])
//				printf("|");
//			else 
//				printf(" ");
//			printf("  ");
//
//			/* right outer wall */
//			if (j == N - 1)
//				printf("|");
//		}
//		printf("\n");
//	}
//
//	/* bottom */
//	for (int j = 0; j < N; ++j) {
//		printf("+");
//		printf("--");
//	}
//	printf("+");
//	printf("\n");
//}

void draw(struct square *m) 
{
	initscr();
	int i = 0, j = 0;	// screen position 
	int row = 0, col = 0; // maze position 
	while(row < N) {
		/* top/bottom walls */
		while(col < N) {
			mvprintw(i,j++,"%s","+");
			if (m[row * N + col].w[0]) {
				mvprintw(i,j,"%s","--");
				j += 2;
			} else {
				mvprintw(i,j,"%s","  ");
				j += 2;
			}
			++col;
		}
		mvprintw(i,j++,"%s","+");
//		mvprintw(i,j,"%s","\n"); 
// 			^ unnecessary; unless we copy to file?

		/* left/right walls */
		col = 0;
		++i;
		j = 0;
		while(col < N) {
			if (m[row * N + col].w[3])
				mvprintw(i,j++,"%s","|");
			else 
				mvprintw(i,j++,"%s"," ");
			mvprintw(i,j,"%s","  ");
			j += 2;
			++col;
		}
		if (col == N)
			mvprintw(i,j++,"%s","|");
//		mvprintw(i,j,"%s","\n");
		col = 0;
		++i;
		j = 0;
		++row;
	}

	/* bottom */
	while (col < N) {
		mvprintw(i,j++,"%s","+");
		mvprintw(i,j,"%s","--");
		j += 2;
		++col;
	}
	mvprintw(i,j,"%s","+");
//	mvprintw(i,j,"%s","\n");

	refresh();
	char c = getch();
	if (c == 'q')
		endwin();
}

void set_top_wall(struct submaze *s, uint8_t build, uint8_t row, uint8_t col)
{
	/* last 4 bits are top, bottom, left, and right walls */
	if (build) {
		s->m[row * N + col].w[0]     = 1; 
		s->m[(row-1) * N + col].w[2] = 1; 
	} else { /* tear wall down */
		s->m[row * N + col].w[0]       = 0;
		s->m[(row - 1) * N + col].w[2] = 0; 
	}
}

void set_left_wall(struct submaze *s, uint8_t build, uint8_t row, uint8_t col)
{
	/* last 4 bits are top, bottom, left, and right walls */
	if (build) {
		s->m[row * N + col].w[3]       = 1; 
		s->m[row * N + (col - 1)].w[1] = 1;
	} else { /* tear wall down */
		s->m[row * N + col].w[3]       = 0;
		s->m[row * N + (col - 1)].w[1] = 0; 
	}
}

void new_submaze(struct submaze *t, struct submaze *s, uint8_t vcut, uint8_t hcut)
{
	/* 0 | 1 
	 * - - -
	 * 2 | 3
	 */
	for (int i = 0; i < 4; ++i)
		t[i] = *s;

	t[0].c = vcut - s->j - 1;
	t[0].r = hcut - s->i - 1;

	t[1].j = vcut;
	t[1].c = s->j + s->c - vcut;
	t[1].r = hcut - s->i - 1;

	t[2].i = hcut;
	t[2].c = vcut - s->j - 1;
	t[2].r = s->i + s->r - hcut;

	t[3].i = hcut;
	t[3].j = vcut;
	t[3].r = s->i + s->r - hcut;
	t[3].c = s->j + s->c - vcut;

	/*	for (int i = 0; i < 4; ++i)
		if (t[i].m[t[i].i * N + t[i].j].sqr == 0)
		t[i].m[t[i].i * N + t[i].j].sqr = i + 1;
	 */
}

void gen_maze(struct submaze *s)
{
	if ( s->r == 0 || s->c == 0)
		return;

	uint8_t hcut = 1 + (rand() % s->r) + s->i;
	uint8_t vcut = 1 + (rand() % s->c) + s->j;

	/* build walls */
	for (int k = s->j; k <= s->j + s->c; ++k)
		set_top_wall(s,1,hcut,k);
	for (int k = s->i; k <= s->i + s->r; ++k) 
		set_left_wall(s,1,k,vcut);

	/* punch holes in walls */
	int no_hole = rand() % 4;
	int hole;
	if (no_hole != 0) { /* top arm */ 
		hole = s->i + (rand() % (hcut - s->i));
		set_left_wall(s,0,hole,vcut);
	}
	if (no_hole != 1) { /* bottom arm */
		hole = hcut + (rand() % (s->i + s->r - hcut + 1));
		set_left_wall(s,0,hole,vcut);
	}
	if (no_hole != 2) { /* right arm */
		hole = vcut + (rand() % (s->j + s->c - vcut + 1));
		set_top_wall(s,0,hcut,hole);
	}
	if (no_hole != 3) { /* left arm */
		hole = s->j + (rand() % (vcut - s->j));
		set_top_wall(s,0,hcut,hole);
	}

	/* make and partition the 4 resulting submazes */
	struct submaze t[4];
	new_submaze(t,s,vcut,hcut);
	for (int i = 0; i < 4; ++i)
		gen_maze(&t[i]);
}

struct map* itinerary(struct square* maze) {
	struct map *trip;
	trip->m = maze;
	trip->n_tbl = 0;
	trip->n_heap = 0;

	if (node_at(1)) {
		trip->start_id = 1;
		trip->start_dir = 1;
	} else {
		trip->start_id = N*(N-1) + 1; // why not 0-index nodes? 
		trip->start_dir = 2;
	}

	if (node_at(N*(N-1))) {
		trip->end_id = N*(N-1);
	} else {
		trip->end_id = 2*N*(N-1);
	}
	return trip;
}
			

int main (int argc, char *argv[] ) {
	srand(atoi(argv[1]));
	struct square maze[N * N] = {};
	struct submaze s_, *s = &s_;
	s->m = maze;
	s->i = 0;
	s->j = 0;
	s->r = N-1;
	s->c = N-1;

	/* build exterior walls */
	for (int k = 0; k < N; ++k) {
		s->m[k].w[0] 		= 1;
		s->m[(N - 1) * N + k].w[2] = 1; 
		s->m[k * N].w[3]		= 1;
		s->m[k * N + N - 1].w[1]   = 1;
	}

	/* build interior walls */
	gen_maze(s);

	/* draw in terminal */
	printf("pencils are go...\n");
	draw(maze);

	struct map *trip = itinerary(s->m);
	struct node **tbl = fastest_path(trip);
	return 0;
}