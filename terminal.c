#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "types.h"
#include "screen.h"
#include "helpers.h"

#define SIZE 100

struct termios config_in_use;
struct termios raw;

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
				log_debug_text("read_input() unknown fucker in control sequence, exiting");
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

static void get_terminal_size(struct cursor_state* state)
{ /* or I just could use ioctl instead of fucking with this */
	char inp[SIZE] = "";
	int idx = 0;
	int ch = 0;
	int lines = 0;
	int columns = 0;

	printf("\033[9999;9999H");
	printf("\033[6n"); /* requests cursor position */
	printf("\033[H");
	
	while ((ch = getchar()) != 'R') { /* R is the end of request's response */
		if (ch == EOF) 
			break;
		
		if (isprint(ch)) {
			if (idx + 1 < SIZE) {
				inp[idx] = ch;
				idx++;
			}
		}
	}
	inp[idx] = '\0';

	if (sscanf(inp, "[%d;%d", &lines, &columns) != 2) {
		printf("Failed to get terminal screen size");
		exit(EXIT_FAILURE);
	} else {
		state->screen_max_lines = lines;
		state->screen_max_columns = columns;
	}
}

void enable_raw_mode(struct cursor_state* state)
{
	if (tcgetattr(STDIN_FILENO, &config_in_use) < 0) {
		printf("Failed to save current terminal config\n");
		exit(EXIT_FAILURE);
	}

	raw = config_in_use;
	printf("\033[2J"); /* clears screen */
	setvbuf(stdout, NULL, _IONBF, 0); /* no stdout buffering */
	/* No idea on most of these flags, but don't touch these fuckers */
	raw.c_iflag &= ~(IGNBRK | BRKINT | ICRNL | INLCR | PARMRK | INPCK | ISTRIP | IXON | IGNCR);
	raw.c_oflag &= ~(OPOST);
	raw.c_lflag &= ~(ECHO | ECHONL | ICANON | IEXTEN | ISIG);
	raw.c_cflag |= (CS8);
	raw.c_cc[VMIN] = 1;
	raw.c_cc[VTIME] = 0;
	
	if (tcsetattr(STDIN_FILENO, TCSANOW, &raw) < 0) {
		printf("Failed to apply raw terminal mode\n");
		exit(EXIT_FAILURE);
	}
	
	get_terminal_size(state);
}

void disable_raw_mode()
{
	printf("\033[H\033[2J");
	if (tcsetattr(STDIN_FILENO, TCSANOW, &config_in_use) < 0) {
		printf("Failed to restore terminal config\n");
		exit(EXIT_FAILURE);
	}
}
