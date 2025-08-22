#ifndef UTILS_H
#define UTILS_H
#include <stddef.h>
#include "types.h"

int read_input(void);
void handle_command_line_args(struct editor_buffer *buf, struct cursor_state *s, int argc, char **argv);
void save_and_exit(struct editor_buffer *buf, char *filename);
void move_cursor(struct editor_buffer *buf, struct cursor_state *s, const int dx, const int dy);
void write_to_buffer(struct editor_buffer *buf, struct cursor_state *s, const int c);
void do_backspace(struct editor_buffer *buf, struct cursor_state *s);
void do_enter(struct editor_buffer *buf, struct cursor_state *s);
void init_editor_buf(struct editor_buffer *buf);

#endif
