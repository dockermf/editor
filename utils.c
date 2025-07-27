#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"


static bool can_move_cursor(CursorState* state, int dx, int dy)
{
	if (state->dx + dx <= 0 || state->dy + dy <= 0) {
		return false;
	} else {
		return true;
	}
}

static void display_cursor_position(CursorState* state)
{
	int offset = 9;
	printf("\033[%i;%iH", state->rows, state->columns - offset);
	printf("%i, %i  ", state->dy, state->dx);
	printf("\033[%i;%iH", state->dy, state->dx);
}

void move_cursor(CursorState* state, int dx, int dy)
{
	if (!can_move_cursor(state, dx, dy)) return;
	state->dx += dx;
	state->dy += dy;
	printf("\033[%i;%iH", state->dy, state->dx);
	display_cursor_position(state);
}

static void arr_increase_capacity(CursorState* state)
{
	size_t new_capacity = state->capacity * 2;
	fprintf(stderr, "next size: %ld\n", new_capacity * sizeof(size_t));
	size_t* new_pointer = realloc(state->lengths, new_capacity * sizeof(size_t));
	if (new_pointer == NULL) {
		fprintf(stderr, "arr_increase_capacity() failed to reallocate memory\n");
		exit(EXIT_FAILURE);
	} else {
		fprintf(stderr, "arr_increase_capacity() allocated %ld bytes of memory\n", new_capacity * sizeof(size_t));
		memset(&state->lengths[state->capacity], 0, state->capacity * sizeof(size_t));
	}
	state->lengths = new_pointer;
	state->capacity = new_capacity;
	fprintf(stderr, "capacity: %ld, size: %ld, allocated: %ld\n", state->capacity, state->size, state->capacity * sizeof(size_t));
}

static void arr_push_char(CursorState* state)
{
	if (state->dy > state->capacity) {
		fprintf(stderr, "tried writing on a new line, but we are out of memory (%ld bytes)\n", state->capacity * sizeof(size_t));
		arr_increase_capacity(state);
	} else {
		int row = state->dy;
		int col = state->dx;
		state->size++;
		state->lengths[row] += 1;
		fprintf(stderr, "length: %ld\n", state->lengths[row]);
	}
}

void init_array(CursorState* state, size_t init_capacity)
{
	state->lengths = (size_t*) malloc(init_capacity * sizeof(size_t));
	state->size = 0;
	state->capacity = init_capacity;
	if (state->lengths == NULL) {
		fprintf(stderr, "init_array() failed to allocate memory\n");
	} else {
		fprintf(stderr, "init_array() successfully allocated memory for %ld lines (%ld bytes while size_t* is %ld bytes)\n", init_capacity, sizeof(state->lengths), sizeof(size_t));
	}
}

void out(CursorState* state, const int inp)
{
	write(1, &inp, 1);
	arr_push_char(state);
	state->dx++;
	display_cursor_position(state);
}

void do_backspace(CursorState* state)
{
	if (!can_move_cursor(state, -1, 0)) return;
	printf("\033[D \033[D");
	state->dx--;
	display_cursor_position(state);
}

void do_enter(CursorState* state)
{
	printf("\033[E");
	state->dx = 1;
	state->dy++;
	display_cursor_position(state);
}

int read_input()
{
	// check ansi table for sequence codes
	char seq[MAX_SEQ_LENGTH];
	ssize_t n = read(STDIN_FILENO, &seq, MAX_SEQ_LENGTH);
	if (seq[0] == '\033' && n >= 3) {
		if (seq[1] == '[') {
			switch (seq[2])
			{
				case 'A': return ARROW_UP_KEY;
				case 'B': return ARROW_DOWN_KEY;
				case 'C': return ARROW_RIGHT_KEY;
				case 'D': return ARROW_LEFT_KEY;
				default: fprintf(stderr, "unknown fucker in control sequence (func read_input)\n");
			}
		}
	} else if (seq[0] == '\177') {
		return BACKSPACE;
	} else if (seq[0] == '\015') {
		return ENTER;
	}
	return seq[0];
}
