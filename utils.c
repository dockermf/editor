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

static size_t get_line_length(struct cursor_state* state, int line)
{
	return state->lengths[line - 1];
}

static size_t* get_line_length_pointer(struct cursor_state* state, int line)
{
	return &state->lengths[line - 1];
}

static bool can_move_cursor(struct cursor_state* state, int dx, int dy)
{
	int col = state->dx;
	int line = state->dy;
	size_t line_length = get_line_length(state, line);
	size_t total_lines = state->total_lines;
	
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
		else if (get_line_length(state, line + dy) >= col)
			return true;
		else if (dy == -1)
			return true;
		else if (dy == 1 && line < total_lines)
		       	return true;
		else
			return false;
	}
}

static bool needs_adjustment(struct cursor_state* state, int dx, int dy)
{
	return get_line_length(state, state->dy + dy) < state->dx;
}

static void display_cursor_position(struct cursor_state* state)
{
	int offset = 9;
	printf("\033[%i;%iH", state->screen_max_lines, state->screen_max_columns - offset);
	printf("%i, %i  ", state->dy, state->dx);
	printf("\033[%i;%iH", state->dy, state->dx);
}

static void update_cursor_position(struct cursor_state* state, int dx, int dy)
{
	state->dx += dx;
	state->dy += dy;
}

static void adjust_pos_to_lastchar(struct cursor_state* state, int dy)
{
	int line = state->dy;
	int next_length = get_line_length(state, line + dy); 
	
	if (next_length == 0)
		state->dx = 1;
	else
		state->dx = next_length;
	update_cursor_position(state, 0, dy);
}

void move_cursor(struct cursor_state* state, int dx, int dy)
{
	if (!can_move_cursor(state, dx, dy))
		return;
	
	if (needs_adjustment(state, dx, dy))
		adjust_pos_to_lastchar(state, dy);
	else
		update_cursor_position(state, dx, dy);

	printf("\033[%i;%iH", state->dy, state->dx);
	display_cursor_position(state);
}

static void print_array_values(size_t* ptr, size_t capacity)
{
	for (int i = 0; i < capacity; i++)
		fprintf(stderr, "[%ld]", ptr[i]);
}

static void init_array_extension(struct cursor_state* state, size_t* arr)
{
	/* TODO: make it support editor array too, or write similar function */
	memset(&arr[state->capacity], 0, state->capacity * sizeof(*arr));
}

static void arr_increase_capacity(struct cursor_state* state)
{
	size_t new_capacity = state->capacity * 2;
	size_t* new_lengths = realloc(state->lengths, new_capacity * sizeof(*new_lengths));
	
	if (!new_lengths) {
		fprintf(stderr, "arr_increase_capacity() failed to reallocate memory\n");
		kill_program();
	}
	
	init_array_extension(state, new_lengths);
	state->lengths = new_lengths;
	state->capacity = new_capacity;
}

static void check_arr_availability(struct cursor_state* state)
{
	while (state->dy > state->capacity) {
		fprintf(stderr, "Calling arr_increase_capacity()\n");
		arr_increase_capacity(state);
	}
}

static void arr_push_char(struct cursor_state* state,
			  struct editor_buffer* buf/*,
			  const int c*/)
{
	/* TODO: wtf is this, kill */
	check_arr_availability(state);
	int line_number = state->dy;
	int col = state->dx;
	size_t* length = get_line_length_pointer(state, line_number);
	//char* line = get_line_pointer(buf, line_number);
	
	//line[line_number][col] = c;
	*length += 1;
	fprintf(stderr, "debug: line %d, length %ld\n", line_number, *length);
}

void init_cursor_state(struct cursor_state* state, size_t init_capacity)
{
	state->total_lines = 1;
	state->capacity = init_capacity;
	state->lengths = calloc(init_capacity, init_capacity * sizeof(*state->lengths));
	
	if (!state->lengths) {
		log_debug_text("init_cursor_state() failed to allocate memory");
		kill_program();
	}
}

static size_t buf_get_line_length(struct editor_buffer* buf, const int line)
{
	return buf->line_lengths[line - 1];
}

static size_t* buf_get_line_length_pointer(struct editor_buffer* buf, const int line)
{
	return &buf->line_lengths[line - 1];
}

static void buf_extend_lines(struct editor_buffer* buf)
{
	log_debug_text("buf_extend_lines() attempting to extend");
	size_t current = buf->total_lines;
	size_t line_length = 2;
	char** new_pointer = realloc(buf->lines, current * 2);
	
	if (!new_pointer) {
		log_debug_text("buf_extend_lines() failed reallocating memory, exiting");
		kill_program();
	}

	for (int i = current; i < current * 2; i++)
		new_pointer[i] = malloc(line_length * sizeof(**new_pointer));
	
	buf->lines = new_pointer;
	buf->total_lines *= 2;
	log_debug_text("buf_extend_lines() success");
}

static void buf_put_char(struct editor_buffer* buf, struct cursor_state* state, const int c)
{
	int col = state->dx;
	int line = state->dy;
	
	if (line > buf->total_lines) {
		log_debug_text("buf_put_char() not enough lines, calling buf_extend_lines()");
		buf_extend_lines(buf);
	}
	/* TODO: fix incorrect write */
	buf->lines[line - 1][col - 1] = c;
	log_debug_text("???????");
	fprintf(stderr, "line %d: %s\n", line, buf->lines[line]);
}

static void init_buf_lines(char** lines, size_t size)
{
	log_debug_text("init_buf_lines() initializing the lines...");
	for (int i = 0; i < size; i++) {
		char* mem = malloc(size * sizeof(**lines));
		if (!mem) {
			log_debug_text("init_buf_lines() failed to allocate memory");
			kill_program();
		}
		lines[i] = mem;
	}
	log_debug_text("init_buf_lines() success");
}

void init_editor_buf(struct editor_buffer* buf)
{
	size_t initial_size = 2;
	size_t* line_lengths = malloc(initial_size * sizeof(*line_lengths));
	char** lines = malloc(initial_size * sizeof(*lines));
	
	if (!line_lengths ||
	    !lines) {
		log_debug_text("init_editor_buf() failed to allocate memory");
		kill_program();
	}
	
	init_buf_lines(lines, initial_size);
	buf->total_lines = initial_size;
	buf->lines = lines;
	buf->line_lengths = line_lengths;
}

static void redraw_screen()
{
	/* TBD */
}

void write_to_buffer(struct cursor_state* state, struct editor_buffer* buf, const int c)
{
	/* TODO: write to buf, then redraw */
	//write(1, &c, 1);
	fprintf(stderr, "lines: %ld\n", buf->total_lines);
	log_debug_text("write_to_buffer() calling buf_put_char()");
	buf_put_char(buf, state, c);
	//arr_push_char(state, buf);
	redraw_screen();
	size_t length = get_line_length(state, state->dy);
	if (state->dx != length)
		fprintf(stderr, "length mismatch: dx is %d while length is %ld\n", state->dx, length);	
	state->dx++;
	display_cursor_position(state);
}

void do_backspace(struct cursor_state* state)
{
	size_t* length = get_line_length_pointer(state, state->dy);

	if (!can_move_cursor(state, -1, 0)) {
		if (*length == 0 && state->dy > 1) {
			adjust_pos_to_lastchar(state, -1);
			move_cursor(state, 1, 0);
			state->total_lines--;
		} else {
			return;
		}
	} else {
		printf("\033[D \033[D");
		*length -= 1;
		state->dx--;
	}
	display_cursor_position(state);
}

void do_enter(struct cursor_state* state)
{
	log_debug_text("pressed enter");
	check_arr_availability(state);
	printf("\033[E");
	state->dx = 1;
	state->dy++;
	/*TODO: actually increase memory for buf->lines on enter press */
	state->total_lines++;
	display_cursor_position(state);
}

int read_input()
{
	// check ansi table for sequence codes
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
