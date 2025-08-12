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

struct cursor_state { 
	size_t* lengths;
	size_t total_lines;
	size_t capacity;
	int screen_max_lines;
	int screen_max_columns;
	int dx;
	int dy;
};

struct editor_buffer {
	char** lines;
	size_t* line_lengths;
	size_t total_lines;
};

#endif
