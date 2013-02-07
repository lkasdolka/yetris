

#include <stdlib.h>
#include <ncurses.h>

#include <sys/time.h>  /* select() */
#include <sys/types.h>
#include <unistd.h>
#include "engine.h"
#include "game.h"

/* /\** Definitions for the input keys -- arbitrary numbers *\/ */
/* enum Input { RIGHT=666, DOWN,  LEFT, ROTATE, ROTATE_BACKW, */
/*              DROP,      PAUSE, QUIT, ERROR,  NONE }; */
/* typedef enum Input Input; */


int engine_screen_init(int width, int height)
{
	engine.screen.width  = width;
	engine.screen.height = height;

	/* Starting ncurses! */
	initscr();

	if (has_colors() == TRUE)
	{
		start_color();
		/* Colors (Enum Name,     Foreground,    Background) */
		init_pair(GREEN_BLACK,   COLOR_GREEN,   COLOR_BLACK);
		init_pair(CYAN_BLACK,    COLOR_CYAN,    COLOR_BLACK);
		init_pair(WHITE_BLACK,   COLOR_WHITE,   COLOR_BLACK);
		init_pair(RED_BLACK,     COLOR_RED,     COLOR_BLACK);
		init_pair(BLUE_BLACK,    COLOR_BLUE,    COLOR_BLACK);
		init_pair(MAGENTA_BLACK, COLOR_MAGENTA, COLOR_BLACK);
		init_pair(YELLOW_BLACK,  COLOR_YELLOW,  COLOR_BLACK);
		init_pair(WHITE_BLACK,   COLOR_WHITE,   COLOR_BLACK);
		init_pair(BLACK_GREEN,   COLOR_BLACK,   COLOR_GREEN);
		init_pair(BLACK_CYAN,    COLOR_BLACK,   COLOR_CYAN);
		init_pair(BLACK_WHITE,   COLOR_BLACK,   COLOR_WHITE);
		init_pair(BLACK_RED,     COLOR_BLACK,   COLOR_RED);
		init_pair(BLACK_BLUE,    COLOR_BLACK,   COLOR_BLUE);
		init_pair(BLACK_MAGENTA, COLOR_BLACK,   COLOR_MAGENTA);
		init_pair(BLACK_YELLOW,  COLOR_BLACK,   COLOR_YELLOW);
		init_pair(BLACK_WHITE,   COLOR_BLACK,   COLOR_WHITE);
	}

	/* Gets the current width and height */
	int current_height, current_width;
	getmaxyx (stdscr, current_height, current_width);

	if ((current_width  < engine.screen.width) ||
	    (current_height < engine.screen.height))
	{
		fprintf(stderr, "Your console screen is smaller than %dx%d\n",
		                 engine.screen.width, engine.screen.height);
		fprintf(stderr, "Please resize your window and try again\n");
		endwin();
		exit(EXIT_FAILURE);
	}

	raw ();       /* Character input doesnt require the <enter> key anymore */
	curs_set (0); /* Makes the blinking cursor invisible */
	noecho ();    /* Wont print the keys received through input */
	nodelay (stdscr, TRUE); /* Wont wait for input */
	keypad (stdscr, TRUE);  /* Support for extra keys (life F1, F2, ... ) */
	refresh ();   /* Refresh the screen (prints whats in the screen buffer) */
}

int engine_windows_init()
{
	window_s  w;
	screen_s* s = &(engine.screen);

	/* main */
	w.width  = 80;
	w.height = 24;
	w.x      = 0;
	w.y      = 0;
	w.win    = newwin(w.height, w.width, w.y, w.x);
	box(w.win, 0, 0);
	wrefresh(w.win);
	s->main = w;

	/* leftmost */
	w.width  = 6 * 2 + 2;
	w.height = s->main.height - 2; /* borders */
	w.x      = 2;
	w.y      = 1;
	w.win    = newwin(w.height, w.width, w.y, w.x);
	wborder(w.win, '|', '|', '-', '-', '+', '+', '+', '+');
	wrefresh(w.win);
	s->leftmost = w;

	/* middle-left */
	w.width  = 10 * 2 + 2;
	w.height = s->main.height - 2; /* borders */
	w.x      = s->leftmost.x + s->leftmost.width + 1;
	w.y      = 1;
	w.win    = newwin(w.height, w.width, w.y, w.x);
	wborder(w.win, '|', '|', '-', '-', '+', '+', '+', '+');
	wrefresh(w.win);
	s->middle_left = w;

	/* middle-right */
	w.width  = 6 * 2 + 2;
	w.height = s->main.height - 2; /* borders */
	w.x      = s->middle_left.x + s->middle_left.width + 1;
	w.y      = 1;
	w.win    = newwin(w.height, w.width, w.y, w.x);
	wborder(w.win, '|', '|', '-', '-', '+', '+', '+', '+');
	wrefresh(w.win);
	s->middle_right = w;

	/* right-most */
	w.width  = s->main.width - (s->middle_right.x + s->middle_right.width) - 3;
	w.height = s->main.height - 2; /* borders */
	w.x      = s->middle_right.x + s->middle_right.width + 1;
	w.y      = 1;
	w.win    = newwin(w.height, w.width, w.y, w.x);
	wborder(w.win, '|', '|', '-', '-', '+', '+', '+', '+');
	wrefresh(w.win);
	s->rightmost = w;

	/* next pieces */
	w.width  = s->middle_right.width  - 2;
	w.height = s->middle_right.height - 2;
	w.x      = 1;
	w.y      = 1;
	w.win    = derwin(s->middle_right.win, w.height, w.width, w.y, w.x);
	wrefresh(w.win);
	s->next_container = w;

	w.width  = s->next_container.width;
	w.height = 5;
	w.x      = 0;
	w.y      = 0;
	w.win    = derwin(s->next_container.win, w.height, w.width, w.y, w.x);
	wrefresh(w.win);
	s->next[0] = w;

	w.width  = s->next_container.width;
	w.height = 5;
	w.x      = 0;
	w.y      = s->next[0].y + s->next[0].height;
	w.win    = derwin(s->next_container.win, w.height, w.width, w.y, w.x);
	wrefresh(w.win);
	s->next[1] = w;

	w.width  = s->next_container.width;
	w.height = 5;
	w.x      = 0;
	w.y      = s->next[1].y + s->next[1].height;
	w.win    = derwin(s->next_container.win, w.height, w.width, w.y, w.x);
	wrefresh(w.win);
	s->next[2] = w;

	w.width  = s->next_container.width;
	w.height = 5;
	w.x      = 0;
	w.y      = s->next[2].y + s->next[2].height;
	w.win    = derwin(s->next_container.win, w.height, w.width, w.y, w.x);
	wrefresh(w.win);
	s->next[3] = w;

	/* game board */
	w.width  = s->middle_left.width  - 2;
	w.height = s->middle_left.height - 2;
	w.x      = 1;
	w.y      = 1;
	w.win    = derwin(s->middle_left.win, w.height, w.width, w.y, w.x);
	wrefresh(w.win);
	s->board = w;

	/* info */
	w.width  = s->leftmost.width  - 2;
	w.height = s->leftmost.height - 2;
	w.x      = 1;
	w.y      = 1;
	w.win    = derwin(s->leftmost.win, w.height, w.width, w.y, w.x);
	wrefresh(w.win);
	s->info = w;

	w = s->info;
	mvwaddstr(w.win, 1, 0, "yetris v0.5");
	mvwaddstr(w.win, 3, 4, "Ahh yeah");
	mvwaddstr(w.win, 4, 4, "This is awesome, everything's fine! "
		                   "you see, this is a multilined string "
		                   "and i'm still able to print it aligned");
	wrefresh(w.win);
}

int engine_init()
{
	/* Block signals while initializing ncurses, otherwise the
	 * console will be screwed up */
	block_signals();

	engine_screen_init(80, 24);
	engine_windows_init();
	engine_keymap(NULL);
	srand(time(NULL));

	restore_signals();
}

int block_signals()
{
//	sigprocmask(SIG_BLOCK);
}

int restore_signals()
{
//	sigprocmask(SIG_SETMASK);
}

void engine_exit()
{
	erase();
	refresh();
	endwin();
}

/** This defines the keymap according to the string #keymap.
 *  Each char represents the key to be pressed for the following
 *  commands (on that order):
 *    down, right, left, rotate, rotate backwards, drop, pause, quit
 *  For example:
 *    "sdawe pq"
 */
int engine_keymap(char keymap[])
{
	/* TODO: check for repeated letters */

	/* ncurses' constants */
	engine.input.none = ERR;

	if ((!keymap) || (strnlen(keymap, 9) != 8))
	{
		/* Invalid string, setting default keymap */
		engine.input.down   = 's';
		engine.input.right  = 'd';
		engine.input.left   = 'a';
		engine.input.rotate = 'w';
		engine.input.rotate_backw = 'e';
		engine.input.drop   = ' ';
		engine.input.pause  = 'p';
		engine.input.quit   = 'q';
		return -1;
	}

	engine.input.down   = keymap[0];
	engine.input.right  = keymap[1];
	engine.input.left   = keymap[2];
	engine.input.rotate = keymap[3];
	engine.input.rotate_backw = keymap[4];
	engine.input.drop   = keymap[5];
	engine.input.pause  = keymap[6];
	engine.input.quit   = keymap[7];
	return 0;
}

/** Get input, waiting at most #delay_ms miliseconds.
 *  @return An input enum - it could be ERROR, you know */
int engine_get_input(int delay_ms)
{
	int retval = 0;
	int c      = 0;
	fd_set input;
	struct timeval timeout;

	timeout.tv_sec = 0;
	timeout.tv_usec = delay_ms * 1000; /* microseconds */

	FD_ZERO(&input);
	FD_SET(STDIN_FILENO, &input);

	/* This function is somewhat complex - check 'man select' for info */
	retval = select(FD_SETSIZE, &input, NULL, NULL, &timeout);
	c = getch();

	if ((retval == 1) && (c == ERR)) /* ERROR */
		return -1;
	if (retval == 0)
		return engine.input.none;

	return c;
}

/* wont call refresh */
void engine_draw_block(block_s* b, WINDOW* w)
{
	wattrset(w, COLOR_PAIR(b->color));
	mvwaddstr(w, b->y, (b->x * 2), b->theme);
}

void engine_draw_piece(piece_s* p, WINDOW* w)
{
	/* WARNING this function assumes there are no more than 4
	 * blocks for each piece on the #pieces global array! */
	int k;
	for (k = 0; k < 4; k++)
		engine_draw_block(&(p->block[k]), w);
}

void engine_draw_board(board_s* b)
{
	WINDOW* w = engine.screen.board.win;

	int i, j;
	for (i = 0; i < BOARD_WIDTH; i++)
		for (j = 0; j < BOARD_HEIGHT; j++)
			if (b->block[i][j].type != EMPTY)
				engine_draw_block(&(b->block[i][j]), w);
}

void engine_draw_next_pieces(game_s* g)
{
	int i, k;
	for (i = 1; i < 5; i++) /* starting at the first next piece */
	{
		piece_s p = g->piece_next[i];
		WINDOW* w = engine.screen.next[i - 1].win;

		werase(w);

		for (k = 0; k < 4; k++)
		{
			p.block[k].x -= p.x;
			p.block[k].y -= p.y;
		}
		engine_draw_piece(&p, w);
		wrefresh(w);
	}
}

/** Calls all drawing routines in order */
void engine_draw(game_s* g)
{
	WINDOW* w = engine.screen.board.win;
	werase(w);

	engine_draw_board(&(g->board));
	engine_draw_piece(&(g->piece_ghost), w);
	engine_draw_piece(g->piece_current, w);
	engine_draw_next_pieces(g);

	wrefresh(w);
}
