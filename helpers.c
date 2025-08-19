#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include "utils.h"
#include "terminal.h"

void log_debug_text(char* text)
{
	fprintf(stderr, "%s\n", text);
}

void kill_program()
{
	disable_raw_mode();
	exit(EXIT_FAILURE);
}

void get_current_directory(char* cwd)
{
	if (getcwd(cwd, MAX_PATH_SIZE) == NULL) {
		log_debug_text("get_current_directory() fatal: couldn't get current directory's path, exiting");
		kill_program();
	}
}

void open_current_directory(char* cwd, DIR** dir)
{
	if ((*dir = opendir(cwd)) == NULL) {
		log_debug_text("open_current_directory() fatal: couldn't open current directory, exiting");
		kill_program();
	}
}

void file_create(char* filename)
{
	log_debug_text("file_create() starting");
	FILE* file_pointer;
	file_pointer = fopen(filename, "w");
	fclose(file_pointer);
	log_debug_text("file_create() finished");
}

void file_write()
{
	
}

bool is_file_present(DIR* dir, char* filename)
{
	struct dirent* e;
	while ((e = readdir(dir)) != NULL) {
		if (strcmp(filename, e->d_name) == 0)
			return true;
	}
	return false;
}

size_t get_line_length(struct editor_buffer* buf, const int line)
{
	return buf->line_lengths[line - 1];
}

size_t* get_line_length_pointer(struct editor_buffer* buf, const int line)
{
	return &buf->line_lengths[line - 1];
}

size_t get_max_line_length(struct editor_buffer* buf, const int line)
{
	return buf->line_max_length[line - 1];
}

size_t* get_max_line_length_pointer(struct editor_buffer* buf, const int line)
{
	return &buf->line_max_length[line - 1];
}

char* get_line_pointer(struct editor_buffer* buf, const int line)
{
	return buf->lines[line - 1];
}

bool can_move_cursor(struct editor_buffer* buf, struct cursor_state* state, const int dx, const int dy)
{
	int col = state->dx;
	int line = state->dy;
	size_t line_length = get_line_length(buf, line);
	size_t total_lines = buf->lines_total;
	
	if (col + dx <= 0 || line + dy <= 0)
		return false;
	
	if (dx != 0) {
		if (col + dx > line_length) {
			if (col + dx - line_length == 1)
				return true;
			else
				return false;
		} else {
			return true;
		}
	} else {
		if (dy == 1 && line == total_lines)
		       	return false;
		else if (get_line_length(buf, line + dy) >= col)
			return true;
		else if (dy == -1)
			return true;
		else if (dy == 1 && line < total_lines)
		       	return true;
		else
			return false;
	}
}

bool needs_adjustment(struct editor_buffer* buf, struct cursor_state* state, int dx, int dy)
{
	return get_line_length(buf, state->dy + dy) < state->dx;
}

void update_cursor_position(struct cursor_state* state, const int dx, const int dy)
{
	state->dx += dx;
	state->dy += dy;
}

void display_cursor_position(struct cursor_state* state)
{
	int offset = 9;
	printf("\033[%i;%iH", state->screen_max_lines, state->screen_max_columns - offset);
	printf("%i, %i  ", state->dy, state->dx);
	printf("\033[%i;%iH", state->dy, state->dx);
}

void adjust_pos_to_lastchar(struct editor_buffer* buf, struct cursor_state* state, const int dy)
{
	int line = state->dy;
	int next_length = get_line_length(buf, line + dy); 
	
	if (next_length == 0)
		state->dx = 1;
	else
		state->dx = next_length;
	update_cursor_position(state, 0, dy);
}

void print_ptr_values(size_t* ptr, const size_t capacity)
{
	for (int i = 0; i < capacity; i++)
		fprintf(stderr, "[%ld]", ptr[i]);
}

void init_ptr_extension(struct editor_buffer* buf, size_t* ptr, size_t value)
{
	for (int i = buf->lines_total; i < buf->lines_total * 2; i++)
		ptr[i] = value;
}

void buf_extend_capacity(struct editor_buffer* buf)
{
	log_debug_text("buf_extend_capacity() attempting to extend");
	size_t new_total = buf->lines_total * 2;
	char** new_lines_pointer = realloc(buf->lines, new_total * sizeof(*new_lines_pointer));
	size_t* new_lengths_pointer = realloc(buf->line_lengths, new_total * sizeof(*new_lengths_pointer));
	size_t* new_max_length_pointer = realloc(buf->line_max_length, new_total * sizeof(*new_max_length_pointer));
	
	if (!new_lines_pointer ||
	    !new_lengths_pointer) {
		log_debug_text("buf_extend_capacity() failed reallocating memory, exiting");
		kill_program();
	}

	for (int i = buf->lines_total; i < new_total; i++)
		new_lines_pointer[i] = malloc(INIT_LINE_LENGTH * sizeof(**new_lines_pointer));
	
	init_ptr_extension(buf, new_lengths_pointer, 0);
	init_ptr_extension(buf, new_max_length_pointer, INIT_LINE_LENGTH);
	
	buf->lines = new_lines_pointer;
	buf->line_lengths = new_lengths_pointer;
	buf->line_max_length = new_max_length_pointer;
	buf->lines_total = new_total;
	
	log_debug_text("buf_extend_capacity() success");
}

void buf_check_capacity(struct editor_buffer* buf, struct cursor_state* state)
{
	if (state->dy > buf->lines_total) {
		log_debug_text("buf_check_capacity() buffer has not enough capacity, calling buf_extend_capacity()");
		buf_extend_capacity(buf);
	}
}

void buf_put_char(struct editor_buffer* buf, struct cursor_state* state, const int c)
{
	int col = state->dx;
	int line = state->dy;
	
	size_t* length_pointer = get_line_length_pointer(buf, line);
	*length_pointer += 1;
	
	if (col < get_line_length(buf, line))
		memmove(&buf->lines[line - 1][col], &buf->lines[line - 1][col - 1], *length_pointer - col);
	
	buf->lines[line - 1][col - 1] = c;
	buf->lines[line - 1][*length_pointer] = '\0';
	
	/* VISUALIZATION
	 * 01234   <- line indexes
	 * test\0
	 *  ^      <- cursor
	 * 12345   <- cursor indexes
	 * teest
	 * tXest\0
	 *   ^
	 *  */
}

void buf_remove_char(struct editor_buffer* buf, struct cursor_state* state)
{
	int line = state->dy;
	int col = state->dx;
	size_t* length_pointer = get_line_length_pointer(buf, line);

	if (col <= *length_pointer)
		memmove(&buf->lines[line - 1][col - 2], &buf->lines[line - 1][col - 1], *length_pointer - col + 1);
	
	*length_pointer -= 1;
	buf->lines[line - 1][*length_pointer] = '\0';
	
	/* VISUALIZATION (removes 1 char left to cursor)
	 * 01234 <- line indexes
	 * test\0
	 *  ^    <- cursor
	 * 12345 <- cursor indexes
	 * estt\0
	 * est\0\0
	 * ^
	 *  */
}

void init_buf_lines(struct editor_buffer* buf, size_t size)
{
	log_debug_text("init_buf_lines() initializing the lines...");
	for (int i = 0; i < size; i++) {
		char* mem = malloc(INIT_LINE_LENGTH * sizeof(**buf->lines));
		
		if (!mem) {
			log_debug_text("init_buf_lines() failed to allocate memory");
			kill_program();
		}
		
		buf->lines[i] = mem;
		buf->line_max_length[i] = INIT_LINE_LENGTH;
	}
	log_debug_text("init_buf_lines() success");
}

void redraw_screen(struct editor_buffer* buf, struct cursor_state* state)
{
	const char* erase_line = "\033[2K";
	const char r_carriage = '\r';

	write(1, &r_carriage, 1);
	write(1, erase_line, strlen(erase_line));

	for (int i = 0; i < get_line_length(buf, state->dy); i++) {
		char c = buf->lines[state->dy - 1][i];
		write(1, &c, 1);
	}
	move_cursor(buf, state, 0, 0);
}

void extend_line_capacity(struct editor_buffer* buf, struct cursor_state* state)
{
	log_debug_text("extend_line_capacity() attempting to extend");
	char* old_line_ptr = get_line_pointer(buf, state->dy);
	size_t* old_max_ptr = get_max_line_length_pointer(buf, state->dy);
	size_t new_max = *old_max_ptr * 2;
	char* new_line_ptr = realloc(old_line_ptr, new_max * sizeof(*new_line_ptr));

	if (!new_line_ptr) {
		log_debug_text("extend_line_capacity() failed reallocating memory, exiting");
		kill_program();
	}
	
	buf->lines[state->dy - 1] = new_line_ptr;
	*old_max_ptr = new_max;
	log_debug_text("extend_line_capacity() success");
}

void check_line_capacity(struct editor_buffer* buf, struct cursor_state* state)
{
	if (get_line_length(buf, state->dy) + 2 > get_max_line_length(buf, state->dy)) {
		log_debug_text("check_line_capacity() calling extend_line_capacity()");
		extend_line_capacity(buf, state);
	}
}
