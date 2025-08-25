#ifndef UTILS_H
#define UTILS_H
#include <stddef.h>
#include "types.h"

void move_cursor(struct cursor_state *s);
void do_backspace(struct editor_buffer *buf, struct cursor_state *s);
void do_enter(struct editor_buffer *buf, struct cursor_state *s);

#endif
