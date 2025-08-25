#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include "buffer.h"
#include "types.h"
#include "utils.h"

void redraw_screen_full(struct editor_buffer *buf, struct cursor_state *state)
{
	const char r_carriage = '\r';
	printf("\033[H\033[2J");

	for (int i = 0; i < buf->lines_total; i++) {
		char *line = buf->lines[i];
		write(1, line, strlen(line));
		write(1, &r_carriage, 1);
	}
	move_cursor(state);
}

void redraw_screen(struct editor_buffer *buf, struct cursor_state *state)
{
	const char *erase_line = "\033[2K";
	const char r_carriage = '\r';

	write(1, &r_carriage, 1);
	write(1, erase_line, strlen(erase_line));

	for (int i = 0; i < get_line_length(buf, state->dy); i++) {
		char c = buf->lines[state->dy - 1][i];
		write(1, &c, 1);
	}
	move_cursor(state);
}
