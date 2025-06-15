#include "mpc/mpc.h"

enum Value { 
	LVAL_NUM,
	LVAL_ERR,
	LVAL_SYM,
	LVAL_SEXPR
};

/* deprecated */
enum Error {
	LERR_DIV_ZERO,
	LERR_BAD_OP,
	LERR_BAD_NUM
};

typedef struct lval {
	int type;
	double num;
	// error and symbol types have some string data
	char* err;
	char* sym;
	// count and pointer to list of lvals
	int count;
	struct lval** cell;
} lval;

/* lval* constructors */
lval* lval_num(double x);
lval* lval_err(char* m);
lval* lval_sym(char* s);
lval* lval_sexpr(void);

/* lval* memory util functions */
lval* lval_add(lval* v, lval* x);
lval* lval_pop(lval* v, int i);
lval* lval_take(lval* v, int i);
void lval_del(lval* v);

/* read and print functions */
lval* lval_read_num(mpc_ast_t* t);
lval* lval_read(mpc_ast_t* t);
void lval_expr_print(lval* v, char open, char close);
void lval_print(lval* v);
void lval_println(lval* v);

/* eval functions */
lval* lval_eval_sexpr(lval* v);
lval* lval_eval(lval* v);
/*
lval* eval(mpc_ast_t* t);
lval* eval_op(char* op, lval* x, lval* y);
*/
lval* builtin_op(lval* a, char* op);

// long power(long x, long y);

