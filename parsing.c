#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "mpc/mpc.h"

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
#include "eval.h"

int main(int argc, char** argv) {
	/* Create some parsers */
	mpc_parser_t* Number = mpc_new("number");
	mpc_parser_t* Symbol = mpc_new("symbol");
	mpc_parser_t* Sexpr = mpc_new("sexpr");
    mpc_parser_t* Qexpr = mpc_new("qexpr");
	mpc_parser_t* Expr = mpc_new("expr");
	mpc_parser_t* Lispy = mpc_new("lispy");

	/* Define them with the following language */
	mpca_lang(MPCA_LANG_DEFAULT,
		"													\
			number: /-?[0-9]+\\.?[0-9]*/;					\
			symbol: \"list\" | \"head\" | \"tail\"          \
                  | \"join\" | \"eval\" | '+' | '-'         \
                  | '*' | '/' | '%' | '^';		            \
			sexpr: '(' <expr>* ')';							\
            qexpr: '{' <expr>* '}';                         \
			expr: <number> | <symbol> | <sexpr> | <qexpr>;	\
			lispy: /^/ <expr>* /$/;							\
		",
		Number,
		Symbol,
		Sexpr,
        Qexpr,
		Expr,
		Lispy
	);

    /* Print version and exit info */
    puts("Lispy Version 0.0.0.0.3\n");
    puts("Press ctrl+c to exit\n");

    /* main loop */
    while (1) {
		// prompt user
		char* input = readline("lispy> ");

		// add input to history
		add_history(input);

		// echo back to user
		// printf("No you're a %s\n", input);

		// attempt to parse
		mpc_result_t r;

		if (mpc_parse("<stdin>", input, Lispy, &r)) {
			// print the AST on success
			// mpc_ast_print(r.output);
			// lval* result = eval(r.output);
			// lval_print(result);
			lval* x = lval_eval(lval_read(r.output));
			lval_println(x);
			lval_del(x);
			// mpc_ast_delete(r.output);
		} else {
			// print the error
			mpc_err_print(r.error);
			mpc_err_delete(r.error);
		}

		// free memory
		free(input);
    }
	mpc_cleanup(6, Number, Symbol, Sexpr, Qexpr, Expr, Lispy);

    return 0;
}
