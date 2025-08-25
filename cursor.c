#include <stdio.h>
#include "buffer.h"
#include "types.h"

bool can_move_cursor(struct editor_buffer *buf, struct cursor_state *state, const int dx, const int dy)
{
	/* TODO: simplify rule logic */
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

bool next_line_has_less_cols(struct editor_buffer *buf, struct cursor_state *state, int dy)
{
	return get_line_length(buf, state->dy + dy) < state->dx;
}

void cursor_update_coords(struct cursor_state *state, const int dx, const int dy)
{
	state->dx += dx;
	state->dy += dy;
}

void display_cursor_position(const struct cursor_state *state)
{
	int offset = 9;
	printf("\033[%i;%iH", state->screen_max_lines, state->screen_max_columns - offset);
	printf("%i, %i  ", state->dy, state->dx);
	printf("\033[%i;%iH", state->dy, state->dx);
}

void adjust_pos_to_lastchar(struct editor_buffer *buf, struct cursor_state *state, const int dy)
{
	int line = state->dy;
	int next_length = get_line_length(buf, line + dy); 
	
	if (next_length == 0)
		state->dx = 1;
	else
		state->dx = next_length;
	cursor_update_coords(state, 0, dy);
}
