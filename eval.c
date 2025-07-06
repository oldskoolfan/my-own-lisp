#include <string.h>
#include "eval.h"

/*
 * construct a pointer to a new Number lval
 */
lval* lval_num(double x) {
	lval* v = malloc(sizeof(lval));
	v->type = LVAL_NUM;
	v->num = x;

	return v;
}

/*
 * contruct a pointer to a new Error lval
 */
lval* lval_err(char* m) {
	lval* v = malloc(sizeof(lval));
	v->type = LVAL_ERR;
	v->err = malloc(strlen(m) + 1);
	strcpy(v->err, m);

	return v;
}

/*
 * construct a pointer to a new Symbol lval
 */
lval* lval_sym(char* s) {
	lval* v = malloc(sizeof(lval));
	v->type = LVAL_SYM;
	v->sym = malloc(strlen(s) + 1);
	strcpy(v->sym, s);

	return v;
}

/*
 * construct a pointer to a new empty Sexpr lval
 */
lval* lval_sexpr(void) {
	lval* v = malloc(sizeof(lval));
	v->type = LVAL_SEXPR;
	v->count = 0;
	v->cell = NULL;

	return v;
}

lval* lval_qexpr(void) {
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_QEXPR;
    v->count = 0;
    v->cell = NULL;

    return v;
}

/*
 * Add a new lval to the list
 */
lval* lval_add(lval* v, lval* x) {
	v->count++;
	v->cell = realloc(v->cell, sizeof(lval*) * v->count);
	v->cell[v->count - 1] = x;

	return v;
}

/*
 * Find and return an lval item at i, adjust list size
 */
lval* lval_pop(lval* v, int i) {
    // find the item at i
    lval* x = v->cell[i];
    
    // shift the memory
    memmove(
        &v->cell[i],
        &v->cell[i + 1],
        sizeof(lval*) * (v->count - i - 1)
    );

    // decrease list count (can this be done above?)
    v->count--;
    
    // reallocate memory used
    v->cell = realloc(v->cell, sizeof(lval*) * v->count);

    return x;
}

/*
 * Find and return an lval item at i, delete list
 */
lval* lval_take(lval* v, int i) {
    lval* x = lval_pop(v, i);
    lval_del(v);

    return x;
}


/*
 * free lval pointer memory
 */
void lval_del(lval* v) {
	switch (v->type) {
		// nothing to do for numbers
		case LVAL_NUM:
			break;
		// for error or symbols, free string data
		case LVAL_ERR:
			free(v->err);
			break;
		case LVAL_SYM:
			free(v->sym);
			break;
		// for Sexpr and Qexpr we need to delete all elements
		case LVAL_SEXPR:
        case LVAL_QEXPR:
			for (int i = 0; i < v->count; i++) {
				lval_del(v->cell[i]);
			}
			// also free the list of pointer memory
			free(v->cell);
			break;
	}

	free(v);
}

lval* lval_read_num(mpc_ast_t* t) {
	errno = 0;
	double x = strtod(t->contents, NULL);
	
	return errno != ERANGE ? lval_num(x) : lval_err("invalid number");
}

lval* lval_read(mpc_ast_t* t) {
	// if symbol or number, return conversion to that type
	if (strstr(t->tag, "number")) {
		return lval_read_num(t);
	}

	if (strstr(t->tag, "symbol")) {
		return lval_sym(t->contents);
	}

	// if root (>) or sexpr then create empty list
	lval* x = NULL;

	if (strcmp(t->tag, ">") == 0 || strstr(t->tag, "sexpr")) {
		x = lval_sexpr();
	}

    if (strstr(t->tag, "qexpr")) {
        x = lval_qexpr();
    }

	// fill this list with any valid expression contained within
	for (int i = 0; i < t->children_num; i++) {
		mpc_ast_t* child = t->children[i];
		char* contents = child->contents;
		char* tag = child->tag;

		if (
			strcmp(contents, "(") == 0 ||
			strcmp(contents, ")") == 0 ||
            strcmp(contents, "{") == 0 ||
            strcmp(contents, "}") == 0 ||
			strcmp(tag, "regex") == 0
		   ) {
			continue;
		}
		
		x = lval_add(x, lval_read(child));	
	}

	return x;
}

void lval_expr_print(lval* v, char open, char close) {
	putchar(open);

	for (int i = 0; i < v->count; i++) {
		// print value contained within
		lval_print(v->cell[i]);

		// don't print trailing space if last element
		if (i != (v->count - 1)) {
			putchar(' ');
		}
	}

	putchar(close);
}

void lval_print(lval* v) {
	switch (v->type) {
		// if type is number, print it
		case LVAL_NUM:
			printf("%g", v->num);
			break;
		// if type is error, print based on error
		case LVAL_ERR:
			/*
			char* err_msg;

			switch (v->err) {
				case LERR_DIV_ZERO:
					err_msg = "Division By Zero!";
					break;
				case LERR_BAD_OP:
					err_msg = "Invalid Operator!";
					break;
				case LERR_BAD_NUM:
					err_msg = "Invalid Number!";
					break;
			}

			printf("Error: %s\n", err_msg);
			*/
			printf("Error: %s", v->err);
			break;
		case LVAL_SYM:
			printf("%s", v->sym);
			break;
		case LVAL_SEXPR:
			lval_expr_print(v, '(', ')');
			break;
        case LVAL_QEXPR:
            lval_expr_print(v, '{', '}');
            break;
	}
}

void lval_println(lval* v) {
	lval_print(v);
	putchar('\n');
}

lval* lval_join(lval* x, lval* y) {
    // for each cell in 'y' add it to 'x'
    while (y->count) {
        x = lval_add(x, lval_pop(y, 0));
    }

    // delete the empty 'y' and return 'x'
    lval_del(y);

    return x;
}

lval* builtin_op(lval* a, char* op) {
    // ensure all args are numbers
    for (int i = 0; i < a->count; i++) {
        if (a->cell[i]->type != LVAL_NUM) {
            lval_del(a);

            return lval_err("Cannot operate on non-number!");
        }
    }
    
    // pop first element
    lval* x = lval_pop(a, 0);

    // if no arguments and sub, then perform unary negation
    if ((strcmp(op, "-") == 0) && a->count == 0) {
        x->num = -x->num;
    }

    // evaluate while still args remaining
    while (a->count > 0) {
        // pop next element
        lval* y = lval_pop(a, 0);

        if (strcmp(op, "+") == 0) {
            x->num += y->num;
        }

        if (strcmp(op, "-") == 0) {
            x->num -= y->num;
        }

        if (strcmp(op, "*") == 0) {
            x->num *= y->num;
        }

        if (strcmp(op, "%") == 0) {
            x->num = fmod(x->num, y->num);
        }

        if (strcmp(op, "^") == 0) {
            x->num = pow(x->num, y->num);
        }

        if (strcmp(op, "/") == 0) {
            if (y->num == 0) {
                lval_del(x);
                lval_del(y);
                x = lval_err("Division by zero!");
                break;
            }
            x->num /= y->num;
        }

        lval_del(y);
    }

    lval_del(a);

    return x;
}

void add_quotes(char* quoted, char* raw) {
    *quoted++ = '\'';

    while ((*quoted = *raw) != '\0') {
        quoted++;
        raw++;
    }

    *quoted = '\'';    
}

char* too_many_args_msg(char* fn_name) {
    char* msg_prefix = "Function ";
    char* msg_suffix = " passed too many arguments!";
    
    // add single quotes to function name
    char* quoted_fn_name = malloc(strlen(fn_name) + 3);
    add_quotes(quoted_fn_name, fn_name);
    
    char* formatted_msg = malloc(strlen(msg_prefix) + strlen(quoted_fn_name) + strlen(msg_suffix) + 1);
    *formatted_msg = '\0';

    return strcat(
        strcat(
            strcat(formatted_msg, msg_prefix),
            quoted_fn_name),
        msg_suffix);
}

lval* builtin_head(lval* a) {
    LASSERT(a, a->count == 1, too_many_args_msg("head"));
    //LASSERT(a, a->count == 1, "Function 'head' passed too many arguments!");
    LASSERT(a, a->cell[0]->type == LVAL_QEXPR, "Function 'head' passed incorrect type!");
    LASSERT(a, a->cell[0]->count != 0, "Function 'head' passed {}!");

    lval* v = lval_take(a, 0);

    while (v->count > 1) {
        lval_del(lval_pop(v, 1));
    }

    return v;
}

lval* builtin_tail(lval* a) {
    LASSERT(a, a->count == 1, "Function 'tail' passed too many arguments!");
    LASSERT(a, a->cell[0]->type == LVAL_QEXPR, "Function 'tail' passed incorrect type!");
    LASSERT(a, a->cell[0]->count != 0, "Function 'tail' passed {}!");
    
    lval* v = lval_take(a, 0);
    lval_del(lval_pop(v, 0));
    
    return v;
}

lval* builtin_list(lval* a) {
    a->type = LVAL_QEXPR;

    return a;
}

lval* builtin_eval(lval* a) {
    LASSERT(a, a->count == 1, "Function 'eval' passed too many arguments!");
    LASSERT(a, a->cell[0]->type == LVAL_QEXPR, "Function 'eval' passed incorrect type!");

    lval* x = lval_take(a, 0);
    x->type = LVAL_SEXPR;

    return lval_eval(x);
}

lval* builtin_join(lval* a) {
    for (int i = 0; i < a->count; i++) {
        LASSERT(a, a->cell[i]->type == LVAL_QEXPR, "Function 'join' passed incorrect type!");
    }

    lval* x = lval_pop(a, 0);
    
    while (a->count) {
        x = lval_join(x, lval_pop(a, 0));
    }

    lval_del(a);
    
    return x;    
}

lval* builtin(lval* a, char* func) {
    if (strcmp("list", func) == 0) {
        return builtin_list(a);
    }
    
    if (strcmp("head", func) == 0) {
        return builtin_head(a);
    }

    if (strcmp("tail", func) == 0) {
        return builtin_tail(a);
    }

    if (strcmp("join", func) == 0) {
        return builtin_join(a);
    }

    if (strcmp("eval", func) == 0) {
        return builtin_eval(a);
    }

    if (strstr("+-/*", func)) {
        return builtin_op(a, func);
    }

    lval_del(a);

    return lval_err("Unknown function!");
}

lval* lval_eval_sexpr(lval* v) {
	// evaluate children
	for (int i = 0; i < v->count; i++) {
		v->cell[i] = lval_eval(v->cell[i]);
	}

	// error checking
	for (int i = 0; i < v->count; i++) {
		if (v->cell[i]->type == LVAL_ERR) {
			return lval_take(v, i);
		}
	}

	// empty expression
	if (v->count == 0) {
		return v;
	}

	// single expression
	if (v->count == 1) {
		return lval_take(v, 0);
	}

	// ensure first element is symbol
	lval* f = lval_pop(v, 0);

	if (f->type != LVAL_SYM) {
		lval_del(f);
		lval_del(v);

		return lval_err("S-expression does not start with symbol!");
	}

	// call builtin with operator
	lval* result = builtin(v, f->sym);
	lval_del(f);

	return result;
}

lval* lval_eval(lval* v) {
	// evaluate Sexpressions
	if (v->type == LVAL_SEXPR) {
		return lval_eval_sexpr(v);
	}

	return v;
}

