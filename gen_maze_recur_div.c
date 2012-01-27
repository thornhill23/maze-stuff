char* gen_maze_recur_div(char *maze, struct submaze *sub, int n)
{
	char *part = maze + sub->x * n + sub->y;
	int horiz_cut = (random() % 2);
	int rnd = random();
	int rnd2 = random();
	int hole;

	if (horiz_cut) {
		int hcut; 
		do {
			hcut= 1 + (rnd % sub->rows) + sub->x;
		for (int j = sub->y + 1; j <= sub->y + sub->cols; ++j)
			maze[hcut * n + j] = '+';
		hole = 1 + (rnd2 % sub->rows);
		part[hcut * n + hole] = ' ';
		char *part1 = gen_part_recur_div(part, hcut - 1, sub->cols, n);
		char *part2 = gen_part_recur_div(part + hcut * n, sub->rows - hcut, sub->cols, n);
	} else {
		int vcut = 1 + (rnd % sub->cols);
		for (int i = 1; i <= sub->rows; ++i)
			part[i * n + vcut] = '+';
		hole = 1 + (rnd2 % sub->cols);
		part[hole * n + vcut] = ' ';
		char *part1 = gen_part_recur_div(part, hcut - 1, sub->cols, n);
		char *part2 = gen_part_recur_div(part + vcut, sub->rows, sub->cols - vcut, n);
	}
		

}
