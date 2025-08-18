#include <stddef.h>

#ifndef MY_DATA_TYPES_H
#define MY_DATA_TYPES_H

#define ARROW_UP_KEY 256
#define ARROW_DOWN_KEY 257
#define ARROW_RIGHT_KEY 258
#define ARROW_LEFT_KEY 259
#define BACKSPACE 010
#define ENTER 015

#define MAX_SEQ_LENGTH 8

#define MAX_PATH_SIZE 256

#define INIT_LINE_LENGTH 256

/**
 * IMPORTANT:
 * Member indexing starts from 0, where line count from 1, so accessing
 * any info related to lines would look like this: .member[line - 1].
 */

struct cursor_state { 
	int screen_max_lines;   /* self explanatory */
	int screen_max_columns; /* self explanatory */
	int dx;                 /* cursor's current X position */
	int dy;                 /* cursor's current Y position */
};

struct editor_buffer {
	char** lines;            /* .lines[i] = line pointer, .lines[i][j] = character in a line */
	size_t* line_max_length; /* used to know how many characters lines[i] can hold, not including '\0' */
	size_t* line_lengths;    /* used to know how long current line is, not including '\0' */
	size_t lines_total;      /* used to know how many pointers .lines can hold, useful for realloc'ing line related member sizes */
};

#endif
