#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "terminal.h"


static void log_debug_text(char* text)
{
	fprintf(stderr, "%s\n", text);
}

static void kill_program()
{
	disable_raw_mode();
	exit(EXIT_FAILURE);
}

static size_t get_line_length(struct editor_buffer* buf, const int line)
{
	return buf->line_lengths[line - 1];
}

static size_t* get_line_length_pointer(struct editor_buffer* buf, const int line)
{
	return &buf->line_lengths[line - 1];
}

static size_t get_max_line_length(struct editor_buffer* buf, const int line)
{
	return buf->line_max_length[line - 1];
}

static size_t* get_max_line_length_pointer(struct editor_buffer* buf, const int line)
{
	return &buf->line_max_length[line - 1];
}

static char* get_line_pointer(struct editor_buffer* buf, const int line)
{
	return buf->lines[line - 1];
}

static bool can_move_cursor(struct editor_buffer* buf, struct cursor_state* state, const int dx, const int dy)
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

static bool needs_adjustment(struct editor_buffer* buf, struct cursor_state* state, int dx, int dy)
{
	return get_line_length(buf, state->dy + dy) < state->dx;
}

static void display_cursor_position(struct cursor_state* state)
{
	int offset = 9;
	printf("\033[%i;%iH", state->screen_max_lines, state->screen_max_columns - offset);
	printf("%i, %i  ", state->dy, state->dx);
	printf("\033[%i;%iH", state->dy, state->dx);
}

static void update_cursor_position(struct cursor_state* state, const int dx, const int dy)
{
	state->dx += dx;
	state->dy += dy;
}

static void adjust_pos_to_lastchar(struct editor_buffer* buf, struct cursor_state* state, const int dy)
{
	int line = state->dy;
	int next_length = get_line_length(buf, line + dy); 
	
	if (next_length == 0)
		state->dx = 1;
	else
		state->dx = next_length;
	update_cursor_position(state, 0, dy);
}

void move_cursor(struct editor_buffer* buf, struct cursor_state* state, const int dx, const int dy)
{
	if (!can_move_cursor(buf, state, dx, dy))
		return;
	
	if (needs_adjustment(buf, state, dx, dy))
		adjust_pos_to_lastchar(buf, state, dy);
	else
		update_cursor_position(state, dx, dy);

	printf("\033[%i;%iH", state->dy, state->dx);
	display_cursor_position(state);
}

static void print_ptr_values(size_t* ptr, const size_t capacity)
{
	for (int i = 0; i < capacity; i++)
		fprintf(stderr, "[%ld]", ptr[i]);
}

static void init_ptr_extension(struct editor_buffer* buf, size_t* ptr, size_t value)
{
	for (int i = buf->lines_total; i < buf->lines_total * 2; i++)
		ptr[i] = value;

}

static void buf_extend_capacity(struct editor_buffer* buf)
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

static void check_buf_capacity(struct editor_buffer* buf, struct cursor_state* state)
{
	if (state->dy > buf->lines_total) {
		log_debug_text("check_buf_capacity() buffer has not enough capacity, calling buf_extend_capacity()");
		buf_extend_capacity(buf);
	}
}

static void buf_put_char(struct editor_buffer* buf, struct cursor_state* state, const int c)
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

static void buf_remove_char(struct editor_buffer* buf, struct cursor_state* state)
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

static void init_buf_lines(struct editor_buffer* buf, size_t size)
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

void init_editor_buf(struct editor_buffer* buf)
{
	size_t initial_size = 10;
	size_t* line_lengths = calloc(initial_size, initial_size * sizeof(*line_lengths));
	size_t* line_max_length = calloc(initial_size, initial_size * sizeof(*line_max_length));
	char** lines = malloc(initial_size * sizeof(*lines));
	
	if (!line_lengths ||
	    !lines) {
		log_debug_text("init_editor_buf() failed to allocate memory");
		kill_program();
	}
	
	buf->lines_total = initial_size;
	buf->lines = lines;
	buf->line_lengths = line_lengths;
	buf->line_max_length = line_max_length;
	init_buf_lines(buf, initial_size);
}

static void redraw_screen(struct editor_buffer* buf, struct cursor_state* state)
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

static void extend_line_capacity(struct editor_buffer* buf, struct cursor_state* state)
{
	log_debug_text("extend_line_capacity() attempting to extend");
	char* old_line_ptr = buf->lines[state->dy - 1];
	size_t* old_max_ptr = &buf->line_max_length[state->dy - 1];
	size_t new_max = *old_max_ptr * 2;
	fprintf(stderr, "ptr: %p\nline: %s\nmax: %ld\nnew max: %ld\ny: %d\n", (void*)old_line_ptr, old_line_ptr, *old_max_ptr, new_max, state->dy);
	char* new_line_ptr = realloc(old_line_ptr, new_max * sizeof(*new_line_ptr));

	if (!new_line_ptr) {
		log_debug_text("extend_line_capacity() failed reallocating memory, exiting");
		kill_program();
	}
	
	buf->lines[state->dy - 1] = new_line_ptr;
	*old_max_ptr = new_max;
	log_debug_text("extend_line_capacity() success");
}

static void check_line_capacity(struct editor_buffer* buf, struct cursor_state* state)
{
	if (get_line_length(buf, state->dy) + 2 > get_max_line_length(buf, state->dy)) {
		log_debug_text("check_line_capacity() calling extend_line_capacity()");
		extend_line_capacity(buf, state);
	}
}

void write_to_buffer(struct editor_buffer* buf, struct cursor_state* state, const int c)
{
	/* TODO: write to buf, then redraw */
	log_debug_text("write_to_buffer() calling check_line_capacity()");
	check_line_capacity(buf, state);

	log_debug_text("write_to_buffer() calling buf_put_char()");
	buf_put_char(buf, state, c);
	fprintf(stderr, "line: %s\n", buf->lines[state->dy - 1]);
	
	//log_debug_text("write_to_buffer() calling redraw_screen()");
	redraw_screen(buf, state);
	
	state->dx += 1;
	display_cursor_position(state);
}

void do_backspace(struct editor_buffer* buf, struct cursor_state* state)
{
	size_t* length = get_line_length_pointer(buf, state->dy);

	if (!can_move_cursor(buf, state, -1, 0)) {
		if (*length == 0 && state->dy > 1) {
			adjust_pos_to_lastchar(buf, state, -1);
			move_cursor(buf, state, 1, 0);
		} else {
			return;
		}
	} else {
		buf_remove_char(buf, state);
		redraw_screen(buf, state);
		state->dx -= 1;
	}
	fprintf(stderr, "line: %s\n", buf->lines[state->dy - 1]);
	display_cursor_position(state);
}

void do_enter(struct editor_buffer* buf, struct cursor_state* state)
{
	log_debug_text("do_enter() called");
	buf_put_char(buf, state, '\n');

	state->dy += 1;
	state->dx = 1;

	check_buf_capacity(buf, state);
	printf("\033[E");
	display_cursor_position(state);
}

int read_input()
{
	/* TODO: make control + key press detection */
	char seq[MAX_SEQ_LENGTH];
	ssize_t n = read(STDIN_FILENO, &seq, MAX_SEQ_LENGTH);
	
	if (seq[0] == '\033' && n >= 3) {
		if (seq[1] == '[') {
			switch (seq[2]) {
			case 'A':
				return ARROW_UP_KEY;
			case 'B':
				return ARROW_DOWN_KEY;
			case 'C':
				return ARROW_RIGHT_KEY;
			case 'D':
				return ARROW_LEFT_KEY;
			default:
				fprintf(stderr, "unknown fucker in control sequence (func read_input)\n");
			}
		}
	} else if (seq[0] == '\177') {
		return BACKSPACE;
	} else if (seq[0] == '\015') {
		return ENTER;
	}
	return seq[0];
}
