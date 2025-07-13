#include "mpc/mpc.h"

#define LASSERT(args, cond, err)    \
    if (!(cond)) {                  \
        lval_del(args);             \
        return lval_err(err);       \
    }

enum Value {
	LVAL_NUM,
	LVAL_ERR,
	LVAL_SYM,
	LVAL_SEXPR,
    LVAL_QEXPR,
};

/* deprecated */
enum Error {
	LERR_DIV_ZERO,
	LERR_BAD_OP,
	LERR_BAD_NUM,
};

enum QexprErrorType {
    ERR_TYPE_TOO_MANY_ARGS,
    ERR_TYPE_INCORRECT_TYPE,
    ERR_TYPE_EMPTY_EXPR,
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
lval* lval_qexpr(void);

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
lval* builtin_head(lval* a);
lval* builtin_tail(lval* a);
lval* builtin_list(lval* a);
lval* builtin_eval(lval* a);
lval* builtin_join(lval* a);

// long power(long x, long y);

