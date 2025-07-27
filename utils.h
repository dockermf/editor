#ifndef UTILS_H
#define UTILS_H
#include <stddef.h>
#include "types.h"

int read_input(void);
void move_cursor(CursorState*, int, int);
void out(CursorState*, const int);
void do_backspace(CursorState*);
void do_enter(CursorState*);
void init_array(CursorState*, size_t);

#endif
