#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <dirent.h>
#include "types.h"
#include "terminal.h"
#include "helpers.h"


void handle_command_line_args(int argc, char* argv[])
{
	bool file_exists;
	char* filename = argv[1];
	char cwd[MAX_PATH_SIZE];
	DIR* dir;

	get_current_directory(cwd);
	open_current_directory(cwd, &dir);

	file_exists = is_file_present(dir, filename);

	if (file_exists) {
		/* load the file into the buffer and draw it in terminal */
		log_debug_text("handle_command_line_args() file exists");
	} else {
		/* create the file to write to */
		log_debug_text("handle_command_line_args() provided file doesn't exist, calling file_create()");
		file_create(filename);
	}

}

void move_cursor(struct editor_buffer* buf, struct cursor_state* state, const int dx, const int dy)
{
	if (!can_move_cursor(buf, state, dx, dy))
		return;
	
	if (needs_adjustment(buf, state, dx, dy))
		adjust_pos_to_lastchar(buf, state, dy);
	else
		update_cursor_position(state, dx, dy);

	printf("\033[%i;%iH", state->dy, state->dx);
	display_cursor_position(state);
}

void init_editor_buf(struct editor_buffer* buf)
{
	size_t initial_size = 10;
	size_t* line_lengths = calloc(initial_size, sizeof(*line_lengths));
	size_t* line_max_length = calloc(initial_size, sizeof(*line_max_length));
	char** lines = malloc(initial_size * sizeof(*lines));
	
	if (!line_lengths ||
	    !lines) {
		log_debug_text("init_editor_buf() failed to allocate memory");
		kill_program();
	}
	
	buf->lines_total = initial_size;
	buf->lines = lines;
	buf->line_lengths = line_lengths;
	buf->line_max_length = line_max_length;
	init_buf_lines(buf, initial_size);
}

void write_to_buffer(struct editor_buffer* buf, struct cursor_state* state, const int c)
{
	log_debug_text("write_to_buffer() calling check_line_capacity()");
	check_line_capacity(buf, state);

	log_debug_text("write_to_buffer() calling buf_put_char()");
	buf_put_char(buf, state, c);
	
	log_debug_text("write_to_buffer() calling redraw_screen()");
	redraw_screen(buf, state);
	
	state->dx += 1;
	display_cursor_position(state);
}

void do_backspace(struct editor_buffer* buf, struct cursor_state* state)
{
	size_t* length = get_line_length_pointer(buf, state->dy);

	if (!can_move_cursor(buf, state, -1, 0)) {
		if (*length == 0 && state->dy > 1) {
			adjust_pos_to_lastchar(buf, state, -1);
			move_cursor(buf, state, 1, 0);
		} else {
			return;
		}
	} else {
		buf_remove_char(buf, state);
		redraw_screen(buf, state);
		state->dx -= 1;
	}
	fprintf(stderr, "line: %s\n", buf->lines[state->dy - 1]);
	display_cursor_position(state);
}

void do_enter(struct editor_buffer* buf, struct cursor_state* state)
{
	log_debug_text("do_enter() called");
	buf_put_char(buf, state, '\n');

	state->dy += 1;
	state->dx = 1;

	buf_check_capacity(buf, state);
	printf("\033[E");
	display_cursor_position(state);
}

int read_input()
{
	char seq[MAX_SEQ_LENGTH];
	ssize_t n = read(STDIN_FILENO, &seq, MAX_SEQ_LENGTH);
	
	if (seq[0] == '\033' && n >= 3) {
		if (seq[1] == '[') {
			switch (seq[2]) {
			case 'A':
				return ARROW_UP_KEY;
			case 'B':
				return ARROW_DOWN_KEY;
			case 'C':
				return ARROW_RIGHT_KEY;
			case 'D':
				return ARROW_LEFT_KEY;
			default:
				log_debug_text("unknown fucker in control sequence (func read_input)");
				return EXIT;
			}
		}
	}

	/* Note: for control character codes lookup ascii control characters, or just do: [ascii] & 0x1F, which */
	/* 	 will give you the exact same control ascii code. */
	switch (seq[0]) {
	case '\177':
		return BACKSPACE;
	case '\015':
		return ENTER;
	case '\021':
		return EXIT;
	default:
		return seq[0];
	}
}
