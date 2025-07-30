#include "terminal.h"
#include "utils.h"
#include "types.h"
#include <unistd.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

int main()
{
	size_t init_line_capacity = 1;
	CursorState state = {
		.dx = 1,
		.dy = 1,
		.rows = 0,
		.columns = 0
	};
	
	init_array(&state, init_line_capacity);

	enable_raw_mode(&state);
	
	while (1) {
		int inp = read_input();
		if (inp == 'X') break;
		switch (inp)
		{
			case ARROW_UP_KEY: move_cursor(&state, 0, -1); break;
			case ARROW_DOWN_KEY: move_cursor(&state, 0, 1); break;
			case ARROW_RIGHT_KEY: move_cursor(&state, 1, 0); break;
			case ARROW_LEFT_KEY: move_cursor(&state, -1, 0); break;
			case BACKSPACE: do_backspace(&state); break;
			case ENTER: do_enter(&state); break;
			default: out(&state, inp);
		}
	}
	disable_raw_mode();
	free(state.lengths);
	fprintf(stderr, "Freed array\n");
	return 0;
}
