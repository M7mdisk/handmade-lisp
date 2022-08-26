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
  return res;
}

long eval_op(long x, char *op, long y) {
  if (strcmp(op, "+") == 0) {
    return x + y;
  }
  if (strcmp(op, "-") == 0) {
    return x - y;
  }
  if (strcmp(op, "*") == 0) {
    return x * y;
  }
  if (strcmp(op, "/") == 0) {
    return x / y;
  }
  if (strcmp(op, "%") == 0) {
    return x % y;
  }
  if (strcmp(op, "^") == 0) {
    return pow(x, y);
  }
  if (strcmp(op, "min") == 0) {
    return x > y ? y : x;
  }
  if (strcmp(op, "max") == 0) {
    return x > y ? x : y;
  }
  return 0;
}

long eval(mpc_ast_t *tree) {

  if (strstr(tree->tag, "number")) {
    return atoi(tree->contents);
  }

  if (tree->children_num == 1)
    return eval(tree->children[0]);

  // Skip the first child, because it is (
  char *op = tree->children[1]->contents;

  long x = eval(tree->children[2]);

  if (tree->children_num == 4 && strcmp(op, "-") == 0) {
    return -x;
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
      long result = eval(r.output);
      printf("%li\n", result);
      mpc_ast_delete(r.output);
    } else {
      mpc_err_print(r.error);
      mpc_err_delete(r.error);
    }
  } while (strcmp(input, ".exit") != 0);

  mpc_cleanup(4, Number, Operator, Expr, Hlisp);
  return 0;
}
