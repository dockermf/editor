#include "terminal.h"
#include "utils.h"
#include "types.h"
#include <unistd.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>


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
		fprintf(stderr, "main() can't feed more than 1 file name, only working with the first\n");

	while (1) {
		int inp = read_input();
		
		if (inp == EXIT)
			break;

		switch (inp) {
		case ARROW_UP_KEY:
			move_cursor(&buf, &state, 0, -1);
			break;
		case ARROW_DOWN_KEY:
			move_cursor(&buf, &state, 0, 1);
			break;
		case ARROW_RIGHT_KEY:
			move_cursor(&buf, &state, 1, 0);
			break;
		case ARROW_LEFT_KEY:
			move_cursor(&buf, &state, -1, 0);
			break;
		case BACKSPACE:
			do_backspace(&buf, &state);
			break;
		case ENTER:
			do_enter(&buf, &state);
			break;
		default:
			write_to_buffer(&buf, &state, inp);
		}
	}
	save_and_exit(&buf, argv[1]);
	/* free array here */
	//fprintf(stderr, "Freed array\n");
	return 0;
}
