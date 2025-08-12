#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static void init_arr(char** ptr)
{
	for (int i = 0; i < 3; i++)
		ptr[i] = malloc(10 * sizeof(**ptr));
}

int main()
{
	char** lines = malloc(3 * sizeof(*lines));
	init_arr(lines);
	
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 10; j++)
			lines[i][j] = 'a';
	}
	lines[0][9] = '\0';
	for (int i = 0; i < 3; i++) {
		printf("lines[%d]: %s %d\n", i, lines[i], strlen(lines[i]));
	}

	return 0;
}
