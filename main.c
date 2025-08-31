#include <unistd.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <assert.h>
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
	if (argc == 1) {
		log_debug_text("main() no argument provided, exiting");
		kill_program();
	}

	struct editor_buffer buf;
	struct cursor_state state = {
		.dx = 1,
		.dy = 1
	};

	init_editor_buf(&buf);
	enable_raw_mode(&state);
	handle_command_line_args(&buf, &state, argc, argv);
	
	if (argc > 2)
		log_debug_text("main() can't feed more than 1 argument to the program, only working with the first one");

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
				redraw_line(&buf, &state);
				cursor_update_coords(&state, -1, 0);
			}
			display_cursor_position(&state);
			break;
		case ENTER:
			if (buf.lines_total < state.dy + 1)
				buf_extend_capacity(&buf);
			
			memmove(&buf.lines[state.dy], &buf.lines[state.dy - 1], (buf.lines_total - state.dy + 1) * sizeof(buf.lines[0]));
			
			char *target = get_line_char_pointer(&buf, state.dx, state.dy);
			char **dest = get_array_byte_pointer(&buf, state.dy + 1);
			
			for (int i = 0; i < buf.lines_total; i++)
				fprintf(stderr, "byte %9d: %p\n", i, (void *)&buf.lines[i]);

			fprintf(stderr, "next ln = %d, &buf.lines[%d] = %p, func = %p, dest: %p\n",\
				state.dy + 1, state.dy + 2, (void *)&buf.lines[state.dy + 2],\
				(void *)get_array_byte_pointer(&buf, state.dy + 1), (void *)dest);
			fprintf(stderr, "dest%10c: %p\ntarg%10c: %p\nline%10c: %p \n",\
				' ', (void *)dest, ' ',  (void *)target, ' ',\
				(void *)get_line_pointer(&buf, state.dy));

			/* TODO: figure this shit out, can't copy the line right now */
			strncpy(*dest, target, (strlen(target) - state.dx + 1) * sizeof(*target));
			for (int i = 1; i <= buf.lines_total; i++)
				fprintf(stderr, "%s\n", get_line_pointer(&buf, i));
			break;
		default:
			if (state.dy > buf.lines_max_written)
				buf.lines_max_written = state.dy;

			check_line_capacity(&buf, &state);
			buf_put_char(&buf, &state, inp);
			redraw_line(&buf, &state);
			cursor_update_coords(&state, 1, 0);
		}
		display_cursor_position(&state);
	}

	save_and_exit(&buf, argv[1]);
	/* free array here */
	
	return 0;
}
