#include <dirent.h>
#include "types.h"

#ifndef MY_BUFFER_H
#define MY_BUFFER_H

void get_current_directory(char* cwd);
void open_current_directory(char* cwd, DIR** directory);
bool is_file_present(DIR* directory, char* filename);
void file_create(char* filename);
void file_read_to_buf(struct editor_buffer *buf, char *filenam);
void file_write_from_buf(struct editor_buffer *buf, char *filename);

size_t get_line_length(struct editor_buffer* buf, const int line);
size_t get_max_line_length(struct editor_buffer* buf, const int line);

size_t* get_line_length_pointer(struct editor_buffer* buf, const int line);
size_t* get_max_line_length_pointer(struct editor_buffer* buf, const int line);

void check_line_capacity(struct editor_buffer* buf, struct cursor_state* s);
void extend_line_capacity(struct editor_buffer* buf, struct cursor_state* s);

char* get_line_pointer(struct editor_buffer* buf, const int line);
char* get_line_char_pointer(const struct editor_buffer *buf, int col, int line);
char **get_array_byte_pointer(const struct editor_buffer *buf, const int line);

void init_ptr_extension(struct editor_buffer* buf, size_t* pointer, size_t value);

void init_editor_buf(struct editor_buffer *buf);
void init_buf_lines(struct editor_buffer* buf, size_t size);
void buf_check_capacity(struct editor_buffer* buf, struct cursor_state* s);
void buf_extend_capacity(struct editor_buffer* buf);
void buf_put_char(struct editor_buffer* buf, struct cursor_state* s, const int ascii_code);
void buf_remove_char(struct editor_buffer* buf, struct cursor_state* s);

#endif
