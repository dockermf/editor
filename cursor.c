#include <stdio.h>
#include "buffer.h"
#include "types.h"


void cursor_move(const struct cursor_state *state)
{
	printf("\033[%i;%iH", state->dy, state->dx);
}

void cursor_update_coords(struct cursor_state *state, const int dx, const int dy)
{
	state->dx += dx;
	state->dy += dy;
}

bool can_move_cursor(struct editor_buffer *buf, struct cursor_state *state, const int dx, const int dy)
{
	int col = state->dx;
	int line = state->dy;
	size_t line_length = get_line_length(buf, line);
	size_t total_lines = buf->lines_total;
	
	if (col + dx <= 0 || line + dy <= 0)
		return false;
	
	if (dy != 0) {
		if (dy == 1 && line == total_lines)
			return false;
	}

	if (dx != 0) {
		if (col + dx > line_length && col != line_length)
			return false;
	}

	return true;
}

bool next_line_has_less_cols(struct editor_buffer *buf, struct cursor_state *state, int dy)
{
	return get_line_length(buf, state->dy + dy) < state->dx;
}

void display_cursor_position(const struct cursor_state *state)
{
	int offset = 9;
	printf("\033[%i;%iH", state->screen_max_lines, state->screen_max_columns - offset);
	printf("%i, %i  ", state->dy, state->dx);
	printf("\033[%i;%iH", state->dy, state->dx);
}
