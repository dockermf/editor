#ifndef UTILS_HELPERS_H
#define UTILS_HELPERS_H

void log_debug_text(char* text);
void kill_program(void);
void handle_command_line_args(struct editor_buffer *buf, struct cursor_state *s, int argc, char **argv);
void save_and_exit(struct editor_buffer *buf, char *filename);

#endif
