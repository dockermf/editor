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
	size_t initialized_lines;
	size_t size;
	size_t capacity;
	int dx;
	int dy;
	int rows;
	int columns;
};

#endif
