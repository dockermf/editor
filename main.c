#include <unistd.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include "terminal.h"
#include "buffer.h"
#include "cursor.h"
#include "screen.h"
#include "helpers.h"
#include "types.h"

/* General TODO list:
 * -Search and fix any bugs with the buffer writing/screen rfreshing.
 * -Add file write/read operations.
 * -Polish everything in general.
 * -Optional:
 *   -Once finished, refactor the code:
 *     -Use arena allocator + gap buffer for insert/delete operations.
 *     -Use different arena allocator to store each line's offset in another arena.
 * */

int main(int argc, char *argv[])
{
	struct editor_buffer buf;
	struct cursor_state state = {
		.dx = 1,
		.dy = 1
	};

	init_editor_buf(&buf);
	enable_raw_mode(&state);
	
	if (argc > 1)
		handle_command_line_args(&buf, &state, argc, argv);
	if (argc > 2)
		log_debug_text("main() illegal to feed more than 1 file name to the program, only working with the first");

	while (1) {
		int inp = read_input();
		
		if (inp == EXIT)
			break;

		switch (inp) {
		case ARROW_UP_KEY:
			if (!can_move_cursor(&buf, &state, 0, -1))
				break;
			
			if (next_line_has_less_cols(&buf, &state, -1))
				state.dx = get_line_length(&buf, state.dy - 1) + 1;

			cursor_update_coords(&state, 0, -1);
			cursor_move(&state);
			break;
		case ARROW_DOWN_KEY:
			if (!can_move_cursor(&buf, &state, 0, 1))
				break;

			if (next_line_has_less_cols(&buf, &state, 1))
				state.dx = get_line_length(&buf, state.dy + 1) + 1;

			cursor_update_coords(&state, 0, 1);
			cursor_move(&state);
			break;
		case ARROW_RIGHT_KEY:
			if (!can_move_cursor(&buf, &state, 1, 0))
				break;

			cursor_update_coords(&state, 1, 0);
			cursor_move(&state);
			break;
		case ARROW_LEFT_KEY:
			if (!can_move_cursor(&buf, &state, -1, 0))
				break;

			cursor_update_coords(&state, -1, 0);
			cursor_move(&state);
			break;
		case BACKSPACE:
			if (!can_move_cursor(&buf, &state, -1, 0)) {
				size_t length = get_line_length(&buf, state.dy);
				
				/* TODO: Currently avoids non-empty lines. Make it move the text on a previous line too. */
				if (length > 0 || state.dy == 1)
					break;
				
				state.dx = get_line_length(&buf, state.dy - 1) + 1;
				cursor_update_coords(&state, 0, -1);
				cursor_move(&state);
			} else {
				buf_remove_char(&buf, &state);
				redraw_screen(&buf, &state);
				cursor_update_coords(&state, -1, 0);
			}
			display_cursor_position(&state);
			break;
		case ENTER:
			/* TODO: Implement. */
			break;
		default:
			if (state.dy > buf.lines_max_written)
				buf.lines_max_written = state.dy;

			check_line_capacity(&buf, &state);
			buf_put_char(&buf, &state, inp);
			redraw_screen(&buf, &state);
			cursor_update_coords(&state, 1, 0);
		}
		display_cursor_position(&state);
	}

	save_and_exit(&buf, argv[1]);
	/* free array here */
	
	return 0;
}
