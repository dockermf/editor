#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include "types.h"
#include "cursor.h"
#include "buffer.h"
#include "terminal.h"
#include "screen.h"
#include "helpers.h"


void move_cursor(struct cursor_state *state)
{
	printf("\033[%i;%iH", state->dy, state->dx);
}

void do_backspace(struct editor_buffer *buf, struct cursor_state *state)
{
	size_t* length = get_line_length_pointer(buf, state->dy);

	if (!can_move_cursor(buf, state, -1, 0)) {
		if (*length == 0 && state->dy > 1) {
			adjust_pos_to_lastchar(buf, state, -1);
			move_cursor(state); // x = 1; y = 0
		} else {
			return;
		}
	} else {
		buf_remove_char(buf, state);
		redraw_screen(buf, state);
		state->dx -= 1;
	}
	display_cursor_position(state);
}

void do_enter(struct editor_buffer *buf, struct cursor_state *state)
{
	/* TODO: copy enter'd text past cursor a line below; move pointer array to one element right, making space for a new one. */
	/*	 copy the text to the new spot in the array; remove the text on the previous line; refresh both lines */
	log_debug_text("do_enter() called");
	buf_check_capacity(buf, state);


	redraw_screen_full(buf, state);
	
	move_cursor(state);
	display_cursor_position(state);
}


