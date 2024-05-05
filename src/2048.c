#include "2048.h"
#include "../libft/libft.h"
#include "../ft_printf/ft_printf.h"
#include <curses.h>
#include <stdlib.h>
#include <ncurses.h>
#include <locale.h>
#include <time.h>
#include <stdlib.h>

void	no_op(void)
{
}

unsigned long long	get_one_sec(void)
{
	time_t						secs1;
	time_t						secs2;
	static unsigned long long	one_sec;

	if (one_sec != 0)
		return (one_sec);
	one_sec = 0;
	secs1 = time(NULL);
	while ((secs2 = time(NULL)) == secs1)
		;
	while (time(NULL) == secs2)
		(no_op(), ++one_sec);
	return (0);
	return (one_sec);
}

void	ft_sleep(double time)
{
	unsigned long long	inc;
	unsigned long long	wait;
	unsigned long long	one_sec;

	one_sec = get_one_sec();
	inc = 0;
	wait = (unsigned long long)((double)one_sec * time);
	while (inc < wait)
		(no_op(), ++inc);
}

unsigned int	get_inc(char **envp)
{
	static unsigned int	inc;

	if (envp != NULL)
		inc = *(unsigned int *)envp;
	return (inc++);
}

t_board	*init_board(int dim)
{
	t_board	*board;
	int		i;
	int		j;
	int		v;

	board = malloc(sizeof(*board));
	board->dim = dim;
	board->cells = malloc(sizeof(*board->cells) * (size_t)board->dim);
	i = -1;
	v = 1;
	while (++i < board->dim)
	{
		board->cells[i] = malloc(sizeof(*board->cells[i]) * (size_t)board->dim);
		j = -1;
		while (++j < board->dim)
		{
			board->cells[i][j] = 0;
			board->cells[i][j] = 2048;
			board->cells[i][j] = 2;
			/* board->cells[i][j] = 1 << v++; */
		}
	}
	board->one_sec = get_one_sec();
	board->new_cell = (t_pos){.x = -1, .y = -1};
	board->move_failed = false;
	board->x = -1;
	board->y = -1;
	board->w = -1;
	board->h = -1;
	board->selected = false;
	return (board);
}

void	print_border_top(int x, int y, int cell_dim)
{
	int	d;

	mvprintw(y, x, BOX_DRAWING_TOP_LEFT);
	d = 0;
	while (++d < FONT_ASPECT_RATIO * cell_dim - 1)
		mvprintw(y, x + d, BOX_DRAWING_HORIZONTAL);
	mvprintw(y, x + FONT_ASPECT_RATIO * cell_dim - 1, BOX_DRAWING_TOP_RIGHT);
}

void	print_border_middle(int x, int y, int cell_dim)
{
	int	d;

	d = 0;
	while (++d < cell_dim - 1)
	{
		mvprintw(y + d, x, BOX_DRAWING_VERTICAL);
		mvprintw(y + d, x + FONT_ASPECT_RATIO * cell_dim - 1, BOX_DRAWING_VERTICAL);
	}
}

void	print_border_bottom(int x, int y, int cell_dim)
{
	int	d;

	mvprintw(y + cell_dim - 1, x, BOX_DRAWING_BOTTOM_LEFT);
	d = 0;
	while (++d < FONT_ASPECT_RATIO * cell_dim - 1)
		mvprintw(y + cell_dim - 1, x + d, BOX_DRAWING_HORIZONTAL);
	mvprintw(y + cell_dim - 1, x + FONT_ASPECT_RATIO * cell_dim - 1, BOX_DRAWING_BOTTOM_RIGHT);
}

void	print_border(int y, int x, int cell_dim, int color)
{
	attron(COLOR_PAIR(color));
	print_border_top(x, y, cell_dim);
	print_border_middle(x, y, cell_dim);
	print_border_bottom(x, y, cell_dim);
	attroff(COLOR_PAIR(color));
}

int	max_width(char *lines_str)
{
	char	**lines;
	int		max_width = 0;

	lines = ft_split(lines_str, '\n');
	while (*lines)
		max_width = ft_max(max_width, (int)ft_utf_8_strlen(*lines++));
	return (max_width);
}

int	print_number_char(int num, int x, int y, int cell_dim, int num_len)
{
	if (num_len > FONT_ASPECT_RATIO * cell_dim - 1 || cell_dim - 2 < 0)
		return (0);
	mvprintw(y + cell_dim / 2, x + FONT_ASPECT_RATIO * cell_dim / 2 - num_len / 2, "%d", num);
	return (1);
}

int	print_numbers_height_n(int num, int x, int y, int cell_dim, int height, char *powers)
{
	char	**small_numbers;
	char	*tmp;
	int		lb = 1;
	int		num_len;
	char	**lines;
	int		line_idx = -1;

	if (cell_dim - 2 < height || num > 2147483647) /* ascii art only until 2^30. 2^31-1 will be rounded to 2^30. */
		return (0);
	tmp = ft_replace_all(powers, "NEWLINE\n", "X");
	small_numbers = ft_split(tmp, 'X');
	free(tmp);
	while (num >>= 1)
		++lb;
	num_len = max_width(small_numbers[lb]);
	if (num_len > FONT_ASPECT_RATIO * cell_dim - 2)
		return (0);
	lines = ft_split(small_numbers[lb], '\n');
	while (++line_idx < height)
		mvprintw(y + cell_dim / 2 - height / 2 + line_idx, x + FONT_ASPECT_RATIO * cell_dim / 2 - num_len / 2, "%s", lines[line_idx]);
	return (1);

}

void	fill_inside_cell(int x, int y, int cell_dim)
{
	int	i;
	int	j;

	i = 0;
	while (++i < cell_dim - 1)
	{
		j = 0;
		while (++j < cell_dim * FONT_ASPECT_RATIO - 1)
			mvprintw(y + i, x + j, " ");
	}
}

/* ret == 0 == no error */
int	print_number(int y, int x, int cell_dim, int num)
{
	int	num_len;
	int	color;
	int	ret;

	if (num < 2) /* simplify! */
		color = 2;
	else if (num < 4)
		color = 3;
	else if (num < 8)
		color = 4;
	else if (num < 16)
		color = 5;
	else if (num < 32)
		color = 6;
	else if (num < 64)
		color = 7;
	else if (num < 128)
		color = 8;
	else if (num < 256)
		color = 9;
	else if (num < 512)
		color = 10;
	else if (num < 1024)
		color = 11;
	else if (num < 2048)
		color = 12;
	else if (num < 4096)
		color = 13;
	else
		color = 14;
	attron(COLOR_PAIR(color));
	fill_inside_cell(x, y, cell_dim);
	num_len = (int)ft_strlen(ft_itoa(num));
	ret = 1;
	if (num == 0)
		ret = 0;
	if (print_numbers_height_n(num, x, y, cell_dim, 16, DOH_NUMBERS))
		ret = 0;
	else if (print_numbers_height_n(num, x, y, cell_dim, 8, DOSREBEL_NUMBERS))
		ret = 0;
	else if (print_numbers_height_n(num, x, y, cell_dim, 6, SHADOW_NUMBERS))
		ret = 0;
	else if (print_numbers_height_n(num, x, y, cell_dim, 4, SMALL_NUMBERS))
		ret = 0;
	else if (print_numbers_height_n(num, x, y, cell_dim, 3, MINI_NUMBERS))
		ret = 0;
	else if (print_number_char(num, x, y, cell_dim, num_len))
		ret = 0;
	attroff(COLOR_PAIR(color));
	return (ret);
}

int	print_number_wrapper(t_board *board, int i, int j, int cell_dim)
{
	int	ret;
	int	y;
	int	x;

	y = board->y + i * cell_dim + board->h / 2 - board->dim * cell_dim / 2;
	x = board->x + j * FONT_ASPECT_RATIO * cell_dim + board->w / 2 - board->dim * cell_dim * FONT_ASPECT_RATIO / 2;
	ret = print_number(y, x, cell_dim, board->cells[i][j]);
	return (ret);
}

void	print_border_wrapper(t_board *board, int i, int j, int cell_dim, int color)
{
	int	y;
	int	x;

	y = board->y + i * cell_dim + board->h / 2 - board->dim * cell_dim / 2;
	x = board->x + j * FONT_ASPECT_RATIO * cell_dim + board->w / 2 - board->dim * cell_dim * FONT_ASPECT_RATIO / 2;
	print_border(y, x, cell_dim, color);
}

void	print_tty_too_small(void)
{
	clear();
	mvprintw(0, 0, "Terminal too small!\nResize it or zoom out!");
	refresh();
}

void	print_borders(t_board *board, int cell_dim)
{
	int	i;
	int	j;

	if (board->move_failed)
	{
		i = -1;
		while (++i < board->dim)
		{
			j = -1;
			while (++j < board->dim)
				print_border_wrapper(board, i, j, cell_dim, 17);
		}
		refresh();
		ft_sleep(0.1);
	}

	i = -1;
	while (++i < board->dim)
	{
		j = -1;
		while (++j < board->dim)
			print_border_wrapper(board, i, j, cell_dim, board->selected ? 18 : 1);
	}

}

int	print_numbers(t_board *board, int cell_dim)
{
	int	i;
	int	j;
	int	new_cell_value;

	i = -1;
	while (++i < board->dim)
	{
		j = -1;
		while (++j < board->dim)
		{
			if (board->new_cell.x == i && board->new_cell.y == j)
			{
				new_cell_value = board->cells[i][j];
				board->cells[i][j] = 0;
				print_number_wrapper(board, i, j, cell_dim);
				board->cells[i][j] = new_cell_value;
			}
			else if (print_number_wrapper(board, i, j, cell_dim))
				return (print_tty_too_small(), 1);
		}
	}
	if (board->new_cell.x != -1 && board->new_cell.y != -1)
	{
		refresh();
		ft_sleep(0.7);
		if (print_number_wrapper(board, board->new_cell.x, board->new_cell.y, cell_dim))
			return (print_tty_too_small(), 1);
	}
	return (0);
}

int	print_board(t_board *board, int x, int y, int w, int h)
{
	int	cell_dim;

	board->x = x;
	board->y = y;
	board->w = w;
	board->h = h;
	if (board->h * FONT_ASPECT_RATIO > board->w)
		cell_dim = board->w / (FONT_ASPECT_RATIO * board->dim);
	else
		cell_dim = board->h / board->dim;

	print_borders(board, cell_dim);
	return (print_numbers(board, cell_dim));
}

void	right(t_board *board)
{
	int	i;
	int	j;

	i = -1;
	while (++i < board->dim)
	{
		j = -1;
		while (++j < board->dim)
			board->cells[i][j]++;
	}
}

void	left(t_board *board)
{
	int	i;
	int	j;

	i = -1;
	while (++i < board->dim)
	{
		j = -1;
		while (++j < board->dim)
			board->cells[i][j]--;
	}
}

/* 0 <= BRIGHT <= 1000 */
int	my_init_color(short color, int r, int g, int b)
{
	return (init_color(
		color,
		(short)(r * BRIGHT / 255),
		(short)(g * BRIGHT / 255),
		(short)(b * BRIGHT / 255)
	));
}

void	ft_setenv(char **envp, const char *name, const char *value)
{
	char	**parts;

	while (*envp)
	{
		parts = ft_split(*envp, '=');
		if (!ft_strcmp(parts[0], name))
		{
			*envp = ft_strjoin((char *)name, ft_strjoin("=", (char *)value));
			break ;
		}
		++envp;
	}
}

void	outputPos(char *str, t_pos pos)
{
	ft_printf("%s", str);
	ft_printf("x = %d, y = %d\n", pos.x, pos.y);
}

int	two_or_four(void)
{
	srand((unsigned int)time(NULL) + get_inc(NULL));
	int two_or_four = rand();
	if(two_or_four % 5 < 3)
		two_or_four = 2;
	else
		two_or_four = 4;
	return (two_or_four);
}

void	setZeroAmount(t_board *board)
{
	board->zero_amount = 0;
	for(int i = 0; i < board->dim; i++)
	{
		for(int j = 0; j < board->dim; j++)
		{
			if(board->cells[i][j] == 0)
				board->zero_amount++;
		}
	}
}

t_pos	findFirstZero(t_board *board, t_pos start)
{
	t_pos	found;
	int	i = start.x;
	int j;

	while(i < board->dim)
	{
		if(i == start.x)
			j = start.y;
		else
			j = 0;
		while(j < board->dim)
		{
			if(board->cells[i][j] == 0)
				return(found.x = i, found.y = j, found);
			j++;
		}
		i++;
	}
	return(found.x = -1, found.y = -1, found);
}

t_pos	incrementPosition(t_board *board, t_pos pos)
{
	if(pos.y < board->dim -1)
		return (pos.y++, pos);
	pos.x++;
	if(pos.x >= board->dim)
		return(pos.x = 0, pos.y = 0, pos);
	pos.y = 0;
	return(pos);
}

t_pos	getRandomZeroPos(t_board *board)
{
	t_pos	ret;
	ret.x = 0;
	ret.y = 0;

	srand((unsigned int)time(NULL) + get_inc(NULL));
	int	randomNum = rand();

	setZeroAmount(board);
	if(board->zero_amount <= 0)
		return (ret.x = -1, ret.y = -1, ret);
	int pos = randomNum % (board->zero_amount) +1;
	for (int i = 0; i < (int)pos; i++)
	{
		ret = findFirstZero(board, ret);
		if(i == (int)pos -1)
			break;
		ret = incrementPosition(board, ret);
	}
	return (ret);
}

void initPosition(t_board *board, t_pos pos)
{
	if (pos.x != -1 && pos.y != -1)
		board->cells[pos.x][pos.y] = two_or_four();
}

void	init_colors(void)
{
	my_init_color(10, 187,173,160); /* border */
	my_init_color(11, 205,192,181); /* bg */
	my_init_color(12, 119,110,101); /* 2 fg */
	my_init_color(13, 238,229,219); /* 2 bg */
	my_init_color(14, 119,110,101); /* 4 fg */
	my_init_color(15, 236,224,201); /* 4 bg */
	my_init_color(16, 249,247,243); /* 8 fg */
	my_init_color(17, 242,177,121); /* 8 bg */
	my_init_color(18, 249,247,243); /* 16 fg */
	my_init_color(19, 244,149,98); /* 16 bg */
	my_init_color(20, 249,247,243); /* 32 fg */
	my_init_color(21, 247,125,96); /* 32 bg */
	my_init_color(22, 249,247,243); /* 64 fg */
	my_init_color(23, 246,94,59); /* 64 bg */
	my_init_color(24, 249,247,243); /* 128 fg */
	my_init_color(25, 236,206,115); /* 128 bg */
	my_init_color(26, 249,247,243); /* 256 fg */
	my_init_color(27, 236,205,98); /* 256 bg */
	my_init_color(28, 249,247,243); /* 512 fg */
	my_init_color(29, 237,200,80); /* 512 bg */
	my_init_color(30, 249,247,243); /* 1024 fg */
	my_init_color(31, 236,196,63); /* 1024 bg */
	my_init_color(32, 249,247,243); /* 2048 fg */
	my_init_color(33, 236,195,45); /* 2048 bg */

	init_pair(1, 10, 10); /* border */
	init_pair(2, 11, 11); /* bg */
	init_pair(3, 12, 13); /* 2 */
	init_pair(4, 14, 15); /* 4 */
	init_pair(5, 16, 17); /* 8 */
	init_pair(6, 18, 19); /* 16 */
	init_pair(7, 20, 21); /* 32 */
	init_pair(8, 22, 23); /* 64 */
	init_pair(9, 24, 25); /* 128 */
	init_pair(10, 26, 27); /* 256 */
	init_pair(11, 28, 29); /* 512 */
	init_pair(12, 30, 31); /* 1024 */
	init_pair(13, 32, 33); /* 2048 */
	init_pair(14, COLOR_BLACK, COLOR_CYAN); /* >= 4096 */

	init_pair(15, COLOR_RED, COLOR_BLACK); /* red text */
	init_pair(16, COLOR_GREEN, COLOR_BLACK); /* green text */
	init_pair(17, COLOR_RED, COLOR_RED); /* border; move failed */
	init_pair(18, COLOR_GREEN, COLOR_GREEN); /* border; selected */
}

void	print_select_board(int lower_border)
{
	char		**lines;
	int			width;
	int			height;
	int try = true;
	int off = 1;
	if (try && lower_border >= 9)
	{
		lines = ft_split(DOSREBEL_SELECT_BOARD, '\n');
		width = max_width(DOSREBEL_SELECT_BOARD);
		height = 8;
		if (width > COLS)
		{
			free_double_str(lines);
			try = true;
		}
		else
			try = false;
	}
	if (try && lower_border >= 7)
	{
		lines = ft_split(ANSISHADOW_SELECT_BOARD, '\n');
		width = max_width(ANSISHADOW_SELECT_BOARD);
		height = 6;
		if (width > COLS)
		{
			free_double_str(lines);
			try = true;
		}
		else
			try = false;
	}
	if (try && lower_border >= 4)
	{
		lines = ft_split(SMALL_SELECT_BOARD, '\n');
		width = max_width(SMALL_SELECT_BOARD);
		height = 3;
		if (width > COLS)
		{
			free_double_str(lines);
			try = true;
		}
		else
			try = false;
	}
	if (try)
	{
		lines = ft_split("SELECT A BOARD\n", '\n');
		width = max_width("SELECT A BOARD\n");
		height = 1;
		if (lower_border < 2)
			off = 0;
		if (width > COLS)
		{
			free_double_str(lines);
			try = true;
		}
		else
			try = false;
	}
	int idx = -1;
	while (lines[++idx])
		mvprintw(
			lower_border - 1 - height - off + idx,
			COLS / 2 - width / 2,
			"%s\n",
			lines[idx]
		);
	free_double_str(lines);
}

int	select_dimension()
{
	t_board		*four_board;
	t_board		*five_board;
	t_board		*six_board;
	int			key;
	int			too_small;
	static int	board_number = 4;
	int			lower_border;

	four_board = init_board(4);
	five_board = init_board(5);
	six_board  = init_board(6);
	switch (board_number)
	{
		case 4:
			four_board->selected = true;
			five_board->selected = false;
			six_board->selected = false;
			break ;
		case 5:
			four_board->selected = false;
			five_board->selected = true;
			six_board->selected = false;
			break ;
		case 6:
			four_board->selected = false;
			five_board->selected = false;
			six_board->selected = true;
			break ;
		default:
			four_board->selected = true;
			five_board->selected = false;
			six_board->selected = false;
			break ;
	}

	clear();

	if (COLS > LINES * FONT_ASPECT_RATIO)
	{
		lower_border = (FONT_ASPECT_RATIO * 4 * LINES - COLS) / (FONT_ASPECT_RATIO * 8);
		print_select_board(lower_border);
		too_small =     print_board(four_board,     COLS / 8 - 1, lower_border, COLS / 4, COLS / (FONT_ASPECT_RATIO * 4));
		if (!too_small)
			too_small = print_board(five_board, 3 * COLS / 8,     lower_border, COLS / 4, COLS / (FONT_ASPECT_RATIO * 4));
		if (!too_small)
			too_small = print_board(six_board,  5 * COLS / 8 + 1, lower_border, COLS / 4, COLS / (FONT_ASPECT_RATIO * 4));
	}
	else
	{
		print_select_board(LINES / 8 - 1);
		too_small =     print_board(four_board, (4 * COLS - FONT_ASPECT_RATIO * LINES) / 8,     LINES / 8 - 1, FONT_ASPECT_RATIO * LINES / 4, LINES / 4);
		if (!too_small)
			too_small = print_board(five_board, (4 * COLS - FONT_ASPECT_RATIO * LINES) / 8, 3 * LINES / 8,     FONT_ASPECT_RATIO * LINES / 4, LINES / 4);
		if (!too_small)
			too_small = print_board(six_board,  (4 * COLS - FONT_ASPECT_RATIO * LINES) / 8, 5 * LINES / 8 + 1, FONT_ASPECT_RATIO * LINES / 4, LINES / 4);
	}

	refresh();
	while ((key = getch()))
	{
		clear();

		if (COLS > LINES * FONT_ASPECT_RATIO)
		{
			switch (key)
			{
				case 'l':
				case KEY_RIGHT:
					if (four_board->selected)
					{
						four_board->selected = false;
						five_board->selected = true;
					}
					else if (five_board->selected)
					{
						five_board->selected = false;
						six_board->selected = true;
					}
					else if (six_board->selected)
					{
						six_board->selected = false;
						four_board->selected = true;
					}
					break ;
				case 'h':
				case KEY_LEFT:
					if (four_board->selected)
					{
						four_board->selected = false;
						six_board->selected = true;
					}
					else if (five_board->selected)
					{
						five_board->selected = false;
						four_board->selected = true;
					}
					else if (six_board->selected)
					{
						six_board->selected = false;
						five_board->selected = true;
					}
					break ;
				case ' ':
				case '\n':
					if (four_board->selected)
						board_number = 4;
					else if (five_board->selected)
						board_number = 5;
					else if (six_board->selected)
						board_number = 6;
					goto return_board;
					break ;
				default:
					break ;
			}
			lower_border = (FONT_ASPECT_RATIO * 4 * LINES - COLS) / (FONT_ASPECT_RATIO * 8);
			print_select_board(lower_border);
			too_small =     print_board(four_board,     COLS / 8 - 1, lower_border, COLS / 4, COLS / (FONT_ASPECT_RATIO * 4));
			if (!too_small)
				too_small = print_board(five_board, 3 * COLS / 8,     lower_border, COLS / 4, COLS / (FONT_ASPECT_RATIO * 4));
			if (!too_small)
				too_small = print_board(six_board,  5 * COLS / 8 + 1, lower_border, COLS / 4, COLS / (FONT_ASPECT_RATIO * 4));
		}
		else
		{
			switch (key)
			{
				case 'j':
				case KEY_DOWN:
					if (four_board->selected)
					{
						four_board->selected = false;
						five_board->selected = true;
					}
					else if (five_board->selected)
					{
						five_board->selected = false;
						six_board->selected = true;
					}
					else if (six_board->selected)
					{
						six_board->selected = false;
						four_board->selected = true;
					}
					break ;
				case 'k':
				case KEY_UP:
					if (four_board->selected)
					{
						four_board->selected = false;
						six_board->selected = true;
					}
					else if (five_board->selected)
					{
						five_board->selected = false;
						four_board->selected = true;
					}
					else if (six_board->selected)
					{
						six_board->selected = false;
						five_board->selected = true;
					}
					break ;
				case ' ':
				case '\n':
					if (four_board->selected)
						board_number = 4;
					else if (five_board->selected)
						board_number = 5;
					else if (six_board->selected)
						board_number = 6;
					goto return_board;
					break ;
				default:
					break ;
			}
			print_select_board(LINES / 8 - 1);
			too_small =     print_board(four_board, (4 * COLS - FONT_ASPECT_RATIO * LINES) / 8,     LINES / 8 - 1, FONT_ASPECT_RATIO * LINES / 4, LINES / 4);
			if (!too_small)
				too_small = print_board(five_board, (4 * COLS - FONT_ASPECT_RATIO * LINES) / 8, 3 * LINES / 8,     FONT_ASPECT_RATIO * LINES / 4, LINES / 4);
			if (!too_small)
				too_small = print_board(six_board,  (4 * COLS - FONT_ASPECT_RATIO * LINES) / 8, 5 * LINES / 8 + 1, FONT_ASPECT_RATIO * LINES / 4, LINES / 4);
		}

		refresh();
		refresh();
	}

return_board:
	destroy_board(four_board);
	destroy_board(five_board);
	destroy_board(six_board);
	return (board_number);
}

int	main(int argc, char **argv, char **envp)
{
	t_board	*board;
	int		key;
	int		dim;

	(void)argc;
	(void)argv;

	/* ncurses setup (compile with -lnursesw)*/
	ft_setenv(envp, "TERM", "xterm-256color");
	get_inc(envp);
	setlocale(LC_ALL, "");
	initscr();
	keypad(stdscr, TRUE);
	noecho();
	cbreak();
	curs_set(0);
	start_color();
	set_escdelay(0);

	init_colors();

	attron(COLOR_PAIR(15));
	mvprintw(0, 0, "Initializing...");
	refresh();
	attroff(COLOR_PAIR(15));

	get_one_sec();

	attron(COLOR_PAIR(16));
	clear();
	mvprintw(0, 0, "Done!");
	refresh();
	ft_sleep(0.1);
	clear();
	attroff(COLOR_PAIR(16));

	while (1)
	{
		dim = select_dimension();
		board = init_board(dim);

		srand((unsigned int)time(NULL) + get_inc(NULL));
		t_pos pos1 = getRandomZeroPos(board);
		srand((unsigned int)time(NULL) + get_inc(NULL));
		t_pos pos2 = getRandomZeroPos(board);
		while(pos2.y != -1 && pos2.x != -1 && pos1.x == pos2.x && pos1.y == pos2.y)
			pos2 = getRandomZeroPos(board);
		initPosition(board, pos1);
		initPosition(board, pos2);

		print_board(board, 0, 0, COLS, LINES);
		while ((key = getch()))
		{
			/* add_to_linked_list(key, time(NULL)); */
			/* if (how_many_keys_have_been_pressed_in_the_last_2_seconds() > 10) */
				/* reduce_global_delay_to_small_value() */
			/* else */
				/* vamp_up() */
			if (key == 'q' || key == 27)
				break ;
			else if (key == KEY_RESIZE)
			{
				board->move_failed = false;
				clear();
			}
			else if (launch_arrows(board, key))
			{
				pos1 = getRandomZeroPos(board);
				board->new_cell = pos1;
				initPosition(board, pos1);
				setZeroAmount(board);
				if(board->zero_amount <= 0 && noMovePossible(board) == true)
					break ;
			}
			else
				board->new_cell = (t_pos){.x = -1, .y = -1};
			print_board(board, 0, 0, COLS, LINES);
			refresh();
			refresh();
		}
		destroy_board(board);
	}

	endwin();
	return (0);
}
