#ifndef UTILS_HELPERS_H
#define UTILS_HELPERS_H

void log_debug_text(char* text);
void kill_program(void);

void get_current_directory(char* cwd);
void open_current_directory(char* cwd, DIR** directory);
bool is_file_present(DIR* directory, char* filename);

size_t get_line_length(struct editor_buffer* buf, const int line);
size_t get_max_line_length(struct editor_buffer* buf, const int line);

size_t* get_line_length_pointer(struct editor_buffer* buf, const int line);
size_t* get_max_line_length_pointer(struct editor_buffer* buf, const int line);

void check_line_capacity(struct editor_buffer* buf, struct cursor_state* s);
void extend_line_capacity(struct editor_buffer* buf, struct cursor_state* s);

char* get_line_pointer(struct editor_buffer* buf, const int line);

bool can_move_cursor(struct editor_buffer* buf, struct cursor_state* s, const int dx, const int dy);
bool needs_adjustment(struct editor_buffer* buf, struct cursor_state* s, const int dx, const int dy);
void adjust_pos_to_lastchar(struct editor_buffer* buf, struct cursor_state* s, const int dy);
void update_cursor_position(struct cursor_state* s, const int dx, const int dy);
void display_cursor_position(struct cursor_state* s);
void redraw_screen(struct editor_buffer*, struct cursor_state*);

void print_ptr_values(size_t* pointer, const size_t size);
void init_ptr_extension(struct editor_buffer* buf, size_t* pointer, size_t value);

void init_buf_lines(struct editor_buffer* buf, size_t size);
void buf_check_capacity(struct editor_buffer* buf, struct cursor_state* s);
void buf_extend_capacity(struct editor_buffer* buf);
void buf_put_char(struct editor_buffer* buf, struct cursor_state* s, const int ascii_code);
void buf_remove_char(struct editor_buffer* buf, struct cursor_state* s);

#endif
