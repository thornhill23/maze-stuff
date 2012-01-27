#include "gen_maze.h"

void draw(struct square *m) 
{
	for (int i = 0; i < N; ++i) {
		/* top/bottom walls */
		for (int j = 0; j < N; ++j) {	
			printf("+");
			if (m[i * N + j].w[0])
				printf("--");
			else 
				printf("  ");
		}
		printf("+");
		printf("\n");

		/* left/right walls */
		for (int j = 0; j < N; ++j) {	
			if (m[i * N + j].w[3])
				printf("|");
			else 
				printf(" ");
			printf("  ");

			/* right outer wall */
			if (j == N - 1)
				printf("|");
		}
		printf("\n");
	}

	/* bottom */
	for (int j = 0; j < N; ++j) {
		printf("+");
		printf("--");
	}
	printf("+");
	printf("\n");
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
		if (t[i].m[t[i].i * t[i].n + t[i].j].sqr == 0)
		t[i].m[t[i].i * t[i].n + t[i].j].sqr = i + 1;
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
	draw(maze);

	return(0);
}
