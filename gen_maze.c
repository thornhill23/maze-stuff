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

struct pos {
	uint8_t i;
	uint8_t j;
};
struct pos id_to_draw_pos(uint16_t id) {
//	if (id > 2*N*(N-1))
	struct pos ij;
	uint8_t row,col; 

	if (id <= N * (N - 1)) { 
		// the potential node is on a vertical edge
		row = id / (N - 1);
		col = id % (N - 1);
		ij.i = 2 * row + 1;
		ij.j = 3 * col;
	} else {
	// the potential node is on a horizontal edge
		col = id / (N - 1);
		row = id % (N - 1);
		ij.i = 2 * row;
		ij.j = 3 * col + 1;
	}
	return ij;
}

void draw(struct square *m, struct tbl *tbl, struct maze *maze) 
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

	// overlay fastest path
	struct pos ij;
	struct node *nb = malloc(sizeof(struct node));
	nb->pre = maze->end_id;
	do {
		nb->id = nb->pre;
		nb = hash(nb, tbl);
		ij = id_to_draw_pos(nb->id);
		if (nb->id > N*(N-1)) // horizontal
			mvprintw(ij.i,ij.j,"%s","* ");
		else // vertical
			mvprintw(ij.i,ij.j,"%s","*");
	} while (nb->id != maze->start_id);

	refresh();
	char c = getch();
	if (c == 'q')
		endwin();
	free(nb);
	return;
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

	/* build exterior walls */
	for (int k = 0; k < N; ++k) {
		s->m[k].w[0] 		= 1;
		s->m[(N - 1) * N + k].w[2] = 1; 
		s->m[k * N].w[3]		= 1;
		s->m[k * N + N - 1].w[1]   = 1;
	}

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

void itinerary(struct square *m,struct maze *maze) {
	maze->m = m;

	if (node_at(1,m))
		maze->start_id = 1;
	else
		maze->start_id = N*(N-1) + 1; // why not 0-index nodes? 

	if (node_at(N*(N-1),m)) 
		maze->end_id = N*(N-1);
	else
		maze->end_id = 2*N*(N-1);

	return;
}
			

int main (int argc, char *argv[] ) {
	if (argc != 2) {
		fprintf(stderr, "usage: %s <arg>\n", argc?argv[0]:"maze-test");
		return 1;
	}

	srand(atoi(argv[1]));
	struct square mz[N * N] = {};
	struct submaze s_, *s = &s_;
	s->m = mz;
	s->i = 0;
	s->j = 0;
	s->r = N-1;
	s->c = N-1;

	gen_maze(s);

	struct maze maze; 
	itinerary(s->m,&maze);

	struct tbl tbl; 
	fastest_path(&maze,&tbl);

	draw(s->m,&tbl,&maze);
	return 0;
}
