#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"

static void log_debug_text(char* text)
{
	fprintf(stderr, "%s\n", text);
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
	size_t total_lines = state->size;
	
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
	printf("\033[%i;%iH", state->rows, state->columns - offset);
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
		exit(EXIT_FAILURE);
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

static void arr_push_char(struct cursor_state* state)
{
	/* TODO: make it push to the editor buffer */
	check_arr_availability(state);
	int line = state->dy;
	size_t* length = get_line_length_pointer(state, line);
	*length += 1;
	fprintf(stderr, "debug: line %d, length %ld\n", line, get_line_length(state, line));
}

void init_cursor_state(struct cursor_state* state, size_t init_capacity)
{
	state->size = 1;
	state->capacity = init_capacity;
	state->lengths = calloc(init_capacity, init_capacity * sizeof(*state->lengths));
	if (!state->lengths) {
		log_debug_text("init_cursor_state() failed to allocate memory");
		exit(EXIT_FAILURE);
	}
}

void init_editor_buf(struct editor_buffer* e)
{
	size_t initial_size = 2;
	size_t* line_lengths = malloc(initial_size * sizeof(*line_lengths));
	char** lines = malloc(initial_size * sizeof(*lines));
	
	if (!line_lengths ||
	    !lines) {
		log_debug_text("init_editor_buf() failed to allocate memory");
		exit(EXIT_FAILURE);
	}
	
	e->total_lines = initial_size;
	e->lines = lines;
	e->line_lengths = line_lengths;
}

void out(struct cursor_state* state, const int inp)
{
	/* TODO: write to buf, then redraw */
	write(1, &inp, 1);
	arr_push_char(state);
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
			state->size--;
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
	check_arr_availability(state);
	printf("\033[E");
	state->dx = 1;
	state->dy++;
	state->size++;
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
