#include <stdio.h>
#include <string.h>

/* If we are compiling on Windows compile these functions */
#ifdef _WIN32

char* readline(char* prompt) {
  fputs(prompt, stdout);
  char* res;
  size_t n;
  ssize_t s = get_line(&res,&n,stdin);
  res[s-1] = '\0';
  return res;
}
void add_history(char* unused) {}

#else

#include <editline/readline.h>
#include <editline/history.h>

#endif
static char* input = NULL;
int main(int argc,char** argv){
  printf("HandyLisp V0.0.1\n");
  printf("Press CTRL+C or type .exit to exit the program\n");

  do {
    input = readline("handy> ");
    add_history(input);
    printf("%s\n",input);

  } while(strcmp(input,".exit") !=0) ;
}

