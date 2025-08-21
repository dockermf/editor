#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* A file for practicing and testing */

void print_lines(FILE* fptr)
{
	char buf[128];
	while (fgets(buf, 64, fptr))
		fprintf(stdout, "%s", buf);
}

void print_address(FILE* fptr)
{
	fprintf(stdout, "Address: %ld\n", ftell(fptr));
}

int main()
{
	/* open the file */
	FILE* fptr = fopen("example.txt", "r");
	
	/* read the the lines */
	print_lines(fptr);
	/* rewind file's pointer position back to the beginning */
	rewind(fptr);
	
	/* close the file */
	fclose(fptr);

	/* write setup */
	fptr = fopen("example.txt", "a");
	fprintf(fptr, "something\n");
	print_lines(fptr);
	fclose(fptr);

	return 0;
}
