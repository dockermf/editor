#include <stdlib.h>
#include <stdio.h>
#include "terminal.h"
#include "buffer.h"
#include "cursor.h"
#include "screen.h"

void log_debug_text(char *text)
{
	fprintf(stderr, "%s\n", text);
}

void kill_program()
{
	disable_raw_mode();
	exit(EXIT_FAILURE);
}

void save_and_exit(struct editor_buffer *buf, char *filename)
{
	file_write_from_buf(buf, filename);
	disable_raw_mode();
}

void handle_command_line_args(struct editor_buffer *buf, struct cursor_state *state, int argc, char *argv[])
{
	char *filename = argv[1];
	char cwd[MAX_PATH_SIZE];
	DIR *dir;

	get_current_directory(cwd);
	open_current_directory(cwd, &dir);

	if (is_file_present(dir, filename)) {
		log_debug_text("handle_command_line_args() calling file_read_to_buf()");
		file_read_to_buf(buf, filename);
		log_debug_text("handle_command_line_args() calling redraw_screen_full()");
		redraw_screen_full(buf, state);
	} else {
		log_debug_text("handle_command_line_args() provided file doesn't exist, calling file_create()");
		file_create(filename);
	}
}
