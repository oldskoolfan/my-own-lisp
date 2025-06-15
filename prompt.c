#include <stdio.h>

// declare a buffer for user input of size 2048
static char input[2048];

int main(int argc, char** argv) {
    // print version and exit info
    puts("Lispy Version 0.0.0.0.1\n");
    puts("Press ctrl+c to exit\n");

    // main loop
    while (1) {
	// prompt user
	fputs("lispy> ", stdout);

	// read a line of user input of max size 2048
	fgets(input, 2048, stdin);

	// echo back to user for now
	printf("No you're a %s", input);
    }

    return 0;
}
