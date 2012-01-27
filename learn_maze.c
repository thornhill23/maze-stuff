#include <gen_maze.h>

void learn_maze(struct square *p, int i, int j, int dir)
{
	/* p for _physical_ maze */
	struct sqr_node m[N * N]; /* model maze */
	m(i * N + j).dir = dir;

	explore(m,i,j);
}

void explore(struct square *p, struct sqr_node *m, int i, int j)
{
	read_sensors(p,m,i,j);
	if (m(i * N + j).w[
}

void read_sensors(struct square *p, struct sqr_node *m, int i, int j)
{
	int rear = (m(i * N + j).dir + 2) % 4;
	for (i = 0; i < 4; ++i)
		if (rear != i)
			m(i * N + j).w[i] = p(i * N + j).w[i];
}

//	dijkstra(m,i,j);
	




