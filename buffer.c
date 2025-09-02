#include <dirent.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "types.h"
#include "helpers.h"
#include "cursor.h"


size_t get_line_length(struct editor_buffer *buf, const int line)
{
	return buf->line_lengths[line - 1];
}

size_t *get_line_length_pointer(struct editor_buffer *buf, const int line)
{
	return &buf->line_lengths[line - 1];
}

size_t get_max_line_length(struct editor_buffer *buf, const int line)
{
	return buf->line_max_length[line - 1];
}

size_t *get_max_line_length_pointer(struct editor_buffer *buf, const int line)
{
	return &buf->line_max_length[line - 1];
}

char *get_line_pointer(struct editor_buffer *buf, const int line)
{
	return buf->lines[line - 1];
}

char **get_array_byte_pointer(const struct editor_buffer *buf, const int line)
{
	return &buf->lines[line - 1];
}

char *get_line_char_pointer(const struct editor_buffer *buf, const int col, const int line)
{
	return &buf->lines[line - 1][col - 1];
}

void init_ptr_extension(struct editor_buffer *buf, size_t *ptr, size_t value)
{
	for (int i = buf->lines_total; i < buf->lines_total * 2; i++)
		ptr[i] = value;
}

void buf_extend_capacity(struct editor_buffer *buf)
{
	log_debug_text("buf_extend_capacity() attempting to extend");
	size_t new_total = buf->lines_total * 2;
	char **new_lines_pointer = realloc(buf->lines, new_total * sizeof(*new_lines_pointer));
	size_t *new_lengths_pointer = realloc(buf->line_lengths, new_total * sizeof(*new_lengths_pointer));
	size_t *new_max_length_pointer = realloc(buf->line_max_length, new_total * sizeof(*new_max_length_pointer));
	
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

void buf_check_capacity(struct editor_buffer *buf, struct cursor_state *state)
{
	if (state->dy > buf->lines_total) {
		log_debug_text("buf_check_capacity() buffer has not enough capacity, calling buf_extend_capacity()");
		buf_extend_capacity(buf);
	}
}

void buf_put_char(struct editor_buffer *buf, struct cursor_state *state, const int c)
{
	int col = state->dx;
	int line = state->dy;
	
	size_t *length_pointer = get_line_length_pointer(buf, line);
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

void buf_remove_char(struct editor_buffer *buf, struct cursor_state *state)
{
	int line = state->dy;
	int col = state->dx;
	size_t *length_pointer = get_line_length_pointer(buf, line);

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

void init_buf_lines(struct editor_buffer *buf, size_t size)
{
	log_debug_text("init_buf_lines() initializing the lines...");
	for (int i = 0; i < size; i++) {
		char *mem = malloc(INIT_LINE_LENGTH * sizeof(**buf->lines));
		
		if (!mem) {
			log_debug_text("init_buf_lines() failed to allocate memory");
			kill_program();
		}
		
		buf->lines[i] = mem;
		buf->line_max_length[i] = INIT_LINE_LENGTH;
	}
}

void init_editor_buf(struct editor_buffer *buf)
{
	size_t initial_size = 10;
	size_t *line_lengths = calloc(initial_size, sizeof(*line_lengths));
	size_t *line_max_length = calloc(initial_size, sizeof(*line_max_length));
	char **lines = malloc(initial_size * sizeof(*lines));
	
	if (!line_lengths ||
	    !lines) {
		log_debug_text("init_editor_buf() failed to allocate memory");
		kill_program();
	}
	
	buf->lines_total = initial_size;
	buf->lines = lines;
	buf->line_lengths = line_lengths;
	buf->line_max_length = line_max_length;
	buf->lines_max_written = 1;
	init_buf_lines(buf, initial_size);
}

void extend_line_capacity(struct editor_buffer *buf, struct cursor_state *state)
{
	log_debug_text("extend_line_capacity() attempting to extend");
	char *old_line_ptr = get_line_pointer(buf, state->dy);
	size_t *old_max_ptr = get_max_line_length_pointer(buf, state->dy);
	size_t new_max = *old_max_ptr * 2;
	char *new_line_ptr = realloc(old_line_ptr, new_max * sizeof(*new_line_ptr));

	if (!new_line_ptr) {
		log_debug_text("extend_line_capacity() failed reallocating memory, exiting");
		kill_program();
	}
	
	buf->lines[state->dy - 1] = new_line_ptr;
	*old_max_ptr = new_max;
	log_debug_text("extend_line_capacity() success");
}

void check_line_capacity(struct editor_buffer *buf, struct cursor_state *state)
{
	if (get_line_length(buf, state->dy) + 2 > get_max_line_length(buf, state->dy)) {
		log_debug_text("check_line_capacity() calling extend_line_capacity()");
		extend_line_capacity(buf, state);
	}
}

void get_current_directory(char *cwd)
{
	if (getcwd(cwd, MAX_PATH_SIZE) == NULL) {
		log_debug_text("get_current_directory() fatal: couldn't get current directory's path, exiting");
		kill_program();
	}
}

void open_current_directory(char *cwd, DIR **dir)
{
	if ((*dir = opendir(cwd)) == NULL) {
		log_debug_text("open_current_directory() fatal: couldn't open current directory, exiting");
		kill_program();
	}
}

void file_create(char *filename)
{
	log_debug_text("file_create() starting");
	FILE *file_pointer = fopen(filename, "w");
	fclose(file_pointer);
	log_debug_text("file_create() finished");
}

void file_read_to_buf(struct editor_buffer *buf, char *filename)
{
	FILE *file_pointer = fopen(filename, "r");
	char tmp[4096];
	int line = 0;
	buf->lines_max_written = 1;

	while (fgets(tmp, sizeof(tmp), file_pointer)) {
		if (buf->lines_total < line)
			buf_extend_capacity(buf);

		/* Note: fgets() reads newlines too, so we do -1 to exclude it */
		strncpy(buf->lines[line], tmp, strlen(tmp) - 1);
		buf->line_lengths[line] = strlen(buf->lines[line]);	
		buf->lines_max_written++;
		line++;
	}

	fclose(file_pointer);
}

void file_write_from_buf(struct editor_buffer *buf, char *filename)
{
	FILE *file_pointer = fopen(filename, "w");

	for (int i = 0; i < buf->lines_total; i++) {
		if (strcmp(buf->lines[i], "") == 0)
			continue;
		
		fprintf(file_pointer, "%s\n", buf->lines[i]);
	}
	fclose(file_pointer);
}

bool is_file_present(DIR *dir, char *filename)
{
	struct dirent *e;
	while ((e = readdir(dir)) != NULL) {
		if (strcmp(filename, e->d_name) == 0)
			return true;
	}
	return false;
}


