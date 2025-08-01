#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"


static bool can_move_cursor(struct cursor_state* state, int dx, int dy)
{ //TODO: fix this shit
	if (state->dx + dx <= 0 || state->dy + dy <= 0) {
		return false;
	} else if (state->dx + dx > state->lengths[state->dy - 1]) {
		fprintf(stderr, "state->dx = %d\ndx = %d\nstate->lengths[state->dy] = %ld\n", state->dx, dx, state->lengths[state->dy - 1]);
		return false;
	} else if (dy == 1 && state->lengths[state->dy - 1] == 0) {
		return false;
	} else {
		return true;
	}
}

static void display_cursor_position(struct cursor_state* state)
{
	int offset = 9;
	printf("\033[%i;%iH", state->rows, state->columns - offset);
	printf("%i, %i  ", state->dy, state->dx);
	printf("\033[%i;%iH", state->dy, state->dx);
}

void move_cursor(struct cursor_state* state, int dx, int dy)
{
	if (!can_move_cursor(state, dx, dy)) return;
	state->dx += dx;
	state->dy += dy;
	printf("\033[%i;%iH", state->dy, state->dx);
	display_cursor_position(state);
}

static void arr_increase_capacity(struct cursor_state* state)
{
	size_t new_capacity = state->capacity * 2;
	fprintf(stderr, "(memory: %ld -> %ld)\n", state->capacity * sizeof(*state->lengths), new_capacity * sizeof(*state->lengths));
	size_t* new_pointer = realloc(state->lengths, new_capacity * sizeof(*new_pointer));
	
	if (!new_pointer) {
		fprintf(stderr, "arr_increase_capacity() failed to reallocate memory\n");
		exit(EXIT_FAILURE);
	}
	fprintf(stderr, "arr_increase_capacity() allocated %ld bytes of memory\n    Before memset: ", new_capacity * sizeof(*new_pointer));
	
	for (int i = 0; i < new_capacity; i++) {
		fprintf(stderr, "[%ld]", new_pointer[i]);
	}
	// use new_pointer instead of state->lengths because it's freed by realloc() by now
	memset(&new_pointer[state->capacity], 0, state->capacity * sizeof(*new_pointer));
	fprintf(stderr, "\n    After memset: ");
	
	for (int i = 0; i < new_capacity; i++) {
		fprintf(stderr, "[%ld]", new_pointer[i]);
	}
	
	state->lengths = new_pointer;
	state->capacity = new_capacity;
	fprintf(stderr, "\n    Size: %ld\n    Capacity: %ld\n    Allocated: %ld bytes\n", state->size, state->capacity, state->capacity * sizeof(*state->lengths));
}

static void arr_push_char(struct cursor_state* state)
{
	if (state->dy > state->capacity) {
		fprintf(stderr, "Calling arr_increase_capacity() ");
		arr_increase_capacity(state);
	}
	int row = state->dy;
	int col = state->dx;
	// array index starts at 0, but row at 1
	state->lengths[row - 1] += 1;
	// fprintf(stderr, "length is now: %ld\n", state->lengths[row - 1]);
}

void init_array(struct cursor_state* state, size_t init_capacity)
{
	state->lengths = (size_t*) calloc(init_capacity, init_capacity * sizeof(*state->lengths));
	state->size = 1;
	state->capacity = init_capacity;
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
