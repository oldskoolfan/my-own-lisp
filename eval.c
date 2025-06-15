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
		// for Sexpr we need to delete all elements
		case LVAL_SEXPR:
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

	// fill this list with any valid expression contained within
	for (int i = 0; i < t->children_num; i++) {
		mpc_ast_t* child = t->children[i];
		char* contents = child->contents;
		char* tag = child->tag;

		if (
			strcmp(contents, "(") == 0 ||
			strcmp(contents, ")") == 0 ||
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
	}
}

void lval_println(lval* v) {
	lval_print(v);
	putchar('\n');
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
/*
lval* eval_op(char* op, lval* x, lval* y) {
	if (x->type == LVAL_ERR) {
		return x;
	}

	if (y->type == LVAL_ERR) {
		return y;
	}

	if (strcmp(op, "+") == 0) {
		return lval_num(x->num + y->num);
	}

	if (strcmp(op, "-") == 0) {
		return lval_num(x->num - y->num);
	}

	if (strcmp(op, "*") == 0) {
		return lval_num(x->num * y->num);
	}

	if (strcmp(op, "/") == 0) {
		return y->num == 0
			? lval_err(LERR_DIV_ZERO)
			: lval_num(x->num / y->num);
	}

	if (strcmp(op, "%") == 0) {
		return lval_num(x->num % y->num);
	}

	if (strcmp(op, "^") == 0) {
		return lval_num(power(x->num, y->num));
	}

	return lval_err(LERR_BAD_OP);
}

long power(long x, long y) {
	long answer = 1;

	do {
		answer *= x;
	} while (--y > 0);

	return answer;
}

lval* eval(mpc_ast_t* t) {
	// if tag contains number, return directly
	if (strstr(t->tag, "number")) {
		// check for conversion error
		errno = 0;
		long x = strtol(t->contents, NULL, 10);

		return errno != ERANGE ? lval_num(x) : lval_err(LERR_BAD_NUM);
	}

	// operator is always the second child
	char* op = t->children[1]->contents;

	// get third child
	lval* x = eval(t->children[2]);

	// iterate remaining children and combine
	int i = 3;
	
	while (strstr(t->children[i]->tag, "expr")) {
		x = eval_op(op, x, eval(t->children[i]));
		i++;
	}
		
	return x;
}
*/

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
	lval* result = builtin_op(v, f->sym);
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

