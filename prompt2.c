#include <stdio.h>
#include <stdlib.h>

#if _WIN32

#include <string.h>

static char buffer[2048];

/* Fake readline function */
char* readline(char* prompt) {
    fputs(prompt, stdout);
    fgets(buffer, 2048, stdin);
    char* cpy = malloc(strlen(buffer) + 1);
    strcpy(cpy, buffer);
    cpy[strlen(cpy) - 1] = '\0';
    
    return cpy;
}

/* Fake add_history function */
void add_history(char* unused) {}

#else

#include <editline/readline.h>
#include <editline/history.h>

#endif

int main(int argc, char** argv) {

    /* Print version and exit info */
    puts("Lispy Version 0.0.0.0.2\n");
    puts("Press ctrl+c to exit\n");

    /* main loop */
    while (1) {
	// prompt user
	char* input = readline("lispy> ");

	// add input to history
	add_history(input);

	// echo back to user
	printf("No you're a %s\n", input);

	// free memory
	free(input);
    }

    return 0;
}
