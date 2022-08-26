#include "mpc.h"
#include <stdio.h>
#include <string.h>

/* If we are compiling on Windows compile these functions */
#ifdef _WIN32
char *readline(char *prompt) {
  fputs(prompt, stdout);
  char *res;
  size_t n;
  ssize_t s = get_line(&res, &n, stdin);
  res[s - 1] = '\0';
  return res;
}
void add_history(char *unused) {}
#else
#include <editline/history.h>
#include <editline/readline.h>
#endif

typedef enum { LVAL_NUM, LVAL_ERR } ValType;

typedef enum { LERR_DIV_ZERO, LERR_BAD_OP, LERR_BAD_NUM } LError;

typedef struct {
  ValType type;
  union {
    long num;
    LError err;
  };
} lval;

lval lval_num(long x) {
  lval a;
  a.num = x;
  a.type = LVAL_NUM;
  return a;
}

lval lval_err(LError x) {
  lval a;
  a.type = LVAL_ERR;
  a.err = x;
  return a;
}

void lval_print(lval v) {
  switch (v.type) {
  /* In the case the type is a number print it */
  /* Then 'break' out of the switch. */
  case LVAL_NUM:
    printf("%li", v.num);
    break;

  /* In the case the type is an error */
  case LVAL_ERR:
    /* Check what type of error it is and print it */
    switch (v.err) {
    case LERR_DIV_ZERO:
      printf("Error: Division By Zero!");
      break;
    case LERR_BAD_OP:
      printf("Error: Invalid Operator!");
      break;
    case LERR_BAD_NUM:
      printf("Error: Invalid Number!");
      break;
    }
    break;
  }
}

void lval_println(lval v) {
  lval_print(v);
  putchar('\n');
}

lval eval_op(lval xval, char *op, lval yval) {
  if (xval.type == LVAL_ERR)
    return xval;
  if (yval.type == LVAL_ERR)
    return yval;
  long x = xval.num;
  long y = yval.num;
  if (strcmp(op, "+") == 0) {
    return lval_num(x + y);
  }
  if (strcmp(op, "-") == 0) {
    return lval_num(x - y);
  }
  if (strcmp(op, "*") == 0) {
    return lval_num(x * y);
  }
  if (strcmp(op, "/") == 0) {
    return y == 0 ? lval_err(LERR_DIV_ZERO) : lval_num(x / y);
  }
  if (strcmp(op, "%") == 0) {
    return y == 0 ? lval_err(LERR_DIV_ZERO) : lval_num(x % y);
  }
  if (strcmp(op, "^") == 0) {
    return lval_num(pow(x, y));
  }
  if (strcmp(op, "min") == 0) {
    return lval_num(x > y ? y : x);
  }
  if (strcmp(op, "max") == 0) {
    return lval_num(x > y ? x : y);
  }
  return lval_err(LERR_BAD_OP);
}

lval eval(mpc_ast_t *tree) {

  if (strstr(tree->tag, "number")) {
    return lval_num(atoi(tree->contents));
  }

  if (tree->children_num == 1)
    return eval(tree->children[0]);

  // Skip the first child, because it is (
  char *op = tree->children[1]->contents;

  lval x = eval(tree->children[2]);

  if (tree->children_num == 4 && strcmp(op, "-") == 0) {
    return lval_num(-x.num);
  }

  for (size_t i = 3; i < tree->children_num - 1; i++) {
    x = eval_op(x, op, eval(tree->children[i]));
  }
  return x;
}

static char *input = NULL;
int main(int argc, char **argv) {
  mpc_parser_t *Number = mpc_new("number");
  mpc_parser_t *Operator = mpc_new("operator");
  mpc_parser_t *Expr = mpc_new("expr");
  mpc_parser_t *Hlisp = mpc_new("hlisp");
  mpca_lang(MPCA_LANG_DEFAULT, "\
      number  : /-?[0-9]+/ ;\
      operator: '+' | '-' | '*' | '/' | '%' | '^' | \"min\" | \"max\" ;\
      expr    : <number> | '(' <operator> <expr>+ ')' ;\
      hlisp   : /^/ <operator> <expr>+ /$/ | <expr> ;\
      ",
            Number, Operator, Expr, Hlisp);

  printf("HandyLisp V0.0.1\n");
  printf("Press CTRL+C or type .exit to exit the program.\n");

  mpc_result_t r;
  do {
    input = readline("handy> ");
    add_history(input);
    if (mpc_parse("<stdin>", input, Hlisp, &r)) {
      // mpc_ast_print(r.output);
      lval result = eval(r.output);
      lval_println(result);
      mpc_ast_delete(r.output);
    } else {
      mpc_err_print(r.error);
      mpc_err_delete(r.error);
    }
  } while (strcmp(input, ".exit") != 0);

  mpc_cleanup(4, Number, Operator, Expr, Hlisp);
  return 0;
}
