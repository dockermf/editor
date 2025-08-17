#include "terminal.h"
#include "utils.h"
#include "types.h"
#include <unistd.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

int main()
{
	struct editor_buffer buf;
	struct cursor_state state = {
		.dx = 1,
		.dy = 1
	};
	
	init_editor_buf(&buf);
	
	enable_raw_mode(&state);
	
	while (1) {
		int inp = read_input();
		
		if (inp == 'X')
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
	disable_raw_mode();
	/* free array here */
	//fprintf(stderr, "Freed array\n");
	return 0;
}
