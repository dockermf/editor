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

static int get_line_length(struct cursor_state* state, int row)
{
	return state->lengths[row - 1];
}

static bool can_move_cursor(struct cursor_state* state, int dx, int dy)
{ //TODO: fix
	int col = state->dx;
	int line = state->dy;
	int line_length = get_line_length(state, line);
	size_t total_lines = state->initialized_lines;
	if (col + dx <= 0 || line + dy <= 0) return false;
	if (dx != 0) {
		if (col + dx > line_length) {
			if (col + dx - line_length == 1)
				return true;
			return false;
		} else {
			return true;
		}
	} else {
		if (dy == 1 && line == total_lines)
		       	return false;
		if (get_line_length(state, line + dy) >= col)
			return true;
		if (dy == -1)
			return true;
		if (dy == 1 && line < total_lines)
		       	return true;
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

static void update_cursor_pos(struct cursor_state* state, int dx, int dy)
{
	state->dx += dx;
	state->dy += dy;
}

static void adjust_pos_to_lastchar(struct cursor_state* state, int dy)
{
	int line = state->dy;
	int next_length = get_line_length(state, line + dy); 
	if (next_length == 0) {
		state->dx = 1;
	} else {
		state->dx = next_length;
	}
	update_cursor_pos(state, 0, dy);
}

void move_cursor(struct cursor_state* state, int dx, int dy)
{
	if (!can_move_cursor(state, dx, dy)) return;
	if (needs_adjustment(state, dx, dy)) {
		adjust_pos_to_lastchar(state, dy);
	} else {
		update_cursor_pos(state, dx, dy);
	}
	printf("\033[%i;%iH", state->dy, state->dx);
	display_cursor_position(state);
}

static void print_array_values(size_t* ptr, size_t capacity)
{
	for (int i = 0; i < capacity; i++) {
		fprintf(stderr, "[%ld]", ptr[i]);
	}
}

static void init_array_extension(struct cursor_state* state, size_t* arr)
{
	memset(&arr[state->capacity], 0, state->capacity * sizeof(*arr));
}

static void arr_increase_capacity(struct cursor_state* state)
{
	size_t new_capacity = state->capacity * 2;
	size_t* new_lengths = realloc(state->lengths, new_capacity * sizeof(*new_lengths));
	//fprintf(stderr, "(memory: %ld -> %ld)\n", state->capacity * sizeof(*state->lengths), new_capacity * sizeof(*state->lengths));
	if (!new_lengths) {
		fprintf(stderr, "arr_increase_capacity() failed to reallocate memory\n");
		exit(EXIT_FAILURE);
	}
	//fprintf(stderr, "arr_increase_capacity() allocated %ld bytes of memory\n", new_capacity * sizeof(*new_lengths));
	init_array_extension(state, new_lengths);
	//fprintf(stderr, "    Array values: ");
	//print_array_values(new_lengths, new_capacity);
	
	state->lengths = new_lengths;
	state->capacity = new_capacity;
	
	//fprintf(stderr, "\n    Size: %ld\n    Capacity: %ld\n    Allocated: %ld bytes\n", state->size, state->capacity, state->capacity * sizeof(*state->lengths));
}

static void arr_push_char(struct cursor_state* state)
{
	while (state->dy > state->capacity) {
		fprintf(stderr, "Calling arr_increase_capacity() ");
		arr_increase_capacity(state);
	}
	int row = state->dy;
	int col = state->dx;
	// array index starts at 0, but row at 1
	state->lengths[row - 1] += 1;
	fprintf(stderr, "debug: row %d, length %d\n", row, get_line_length(state, row));
}

void init_array(struct cursor_state* state, size_t init_capacity)
{
	state->lengths = (size_t*) calloc(init_capacity, init_capacity * sizeof(*state->lengths));
	state->size = 1;
	state->capacity = init_capacity;
	state->initialized_lines = 1;
	if (state->lengths == NULL) {
		fprintf(stderr, "init_array() failed to allocate memory\n");
	} else {
		fprintf(stderr, "init_array() successfully allocated memory for %ld lines (%ld bytes while size_t* is %ld bytes)\n", init_capacity, sizeof(state->lengths), sizeof(*state->lengths));
	}
}

void out(struct cursor_state* state, const int inp)
{
	write(1, &inp, 1);
	arr_push_char(state);
	state->dx++;
	if (state->dx - 1 != get_line_length(state, state->dy)) fprintf(stderr, "dx = %d while length = %ld\n", state->dx - 1, state->lengths[state->dy - 1]);	
	display_cursor_position(state);
}

void do_backspace(struct cursor_state* state)
{
	if (!can_move_cursor(state, -1, 0)) return;
	printf("\033[D \033[D");
	state->dx--;
	state->lengths[state->dy - 1]--;
	display_cursor_position(state);
}

void do_enter(struct cursor_state* state)
{
	printf("\033[E");
	state->dx = 1;
	state->dy++;
	state->size++;
	state->initialized_lines++;
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
