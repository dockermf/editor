#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
#include "types.h"

#define SIZE 100

struct termios config_in_use;
struct termios raw;

static void get_terminal_size(CursorState* state)
{ //or you could use ioctl instead of fucking with this
	char inp[SIZE] = "";
	int idx = 0;
	int ch = 0;
	int rows = 0;
	int columns = 0;

	printf("\033[9999;9999H");
	printf("\033[6n"); // request cursor pos
	printf("\033[H");
	
	while ((ch = getchar()) != 'R') { // R is the end of request response
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

	if (sscanf(inp, "[%d;%d", &rows, &columns) != 2) {
		printf("Failed to get terminal screen size");
	} else {
		state->rows = rows;
		state->columns = columns;
	}
}

void enable_raw_mode(CursorState* state)
{
	if (tcgetattr(STDIN_FILENO, &config_in_use) < 0) {
		printf("Failed to save current terminal config\n");
	}
	raw = config_in_use;
	printf("\033[2J"); //clears screen
	setvbuf(stdout, NULL, _IONBF, 0); //no stdout buffering
	/* No idea on most of these flags, but don't touch these fuckers */
	raw.c_iflag &= ~(IGNBRK | BRKINT | ICRNL | INLCR | PARMRK | INPCK | ISTRIP | IXON | IGNCR);
	raw.c_oflag &= ~(OPOST);
	raw.c_lflag &= ~(ECHO | ECHONL | ICANON | IEXTEN | ISIG);
	raw.c_cflag |= (CS8);
	raw.c_cc[VMIN] = 1;
	raw.c_cc[VTIME] = 0;
	
	if (tcsetattr(STDIN_FILENO, TCSANOW, &raw) < 0) {
		printf("Failed to apply raw terminal mode\n");
	}
	get_terminal_size(state);
}

void disable_raw_mode()
{
	printf("\033[H\033[2J");
	if (tcsetattr(STDIN_FILENO, TCSANOW, &config_in_use) < 0) {
		printf("Failed to restore terminal config\n");
	}
}
