#ifndef UTILS_H
#define UTILS_H
#include <stddef.h>
#include "types.h"

int read_input(void);
void move_cursor(struct cursor_state*, int, int);
void out(struct cursor_state*, const int);
void do_backspace(struct cursor_state*);
void do_enter(struct cursor_state*);
void init_cursor_state(struct cursor_state*, size_t);
void init_editor_buf(struct editor_buffer*);

#endif
