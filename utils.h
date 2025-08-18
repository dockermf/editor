#ifndef UTILS_H
#define UTILS_H
#include <stddef.h>
#include "types.h"

int read_input(void);
void handle_command_line_args(int, char**);
void move_cursor(struct editor_buffer*, struct cursor_state*, const int, const int);
void write_to_buffer(struct editor_buffer*, struct cursor_state*, const int);
void do_backspace(struct editor_buffer*, struct cursor_state*);
void do_enter(struct editor_buffer*, struct cursor_state*);
void init_editor_buf(struct editor_buffer*);

#endif
