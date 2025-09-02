#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include "buffer.h"
#include "types.h"
#include "cursor.h"

void redraw_screen_full(struct editor_buffer *buf, struct cursor_state *state)
{
	const char r_carriage = '\r';
	const char newline = '\n';
	printf("\033[H\033[2J");

	for (int i = 0; i < buf->lines_total; i++) {
		const char *line = buf->lines[i];
		write(1, line, strlen(line));
		write(1, &r_carriage, 1);
		write(1, &newline, 1);
	}
	/* Note: this moves cursor back to it's original place */
	cursor_move(state);
}

void redraw_line(struct editor_buffer *buf, struct cursor_state *state)
{
	const char *erase_line = "\033[2K";
	const char r_carriage = '\r';
	const char* line = get_line_pointer(buf, state->dy);

	write(1, &r_carriage, 1);
	write(1, erase_line, strlen(erase_line));
	write(1, line, strlen(line));
	/* Note: this moves cursor back to it's original place */
	cursor_move(state);
}
