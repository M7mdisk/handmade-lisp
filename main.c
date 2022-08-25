#include <stdio.h>
#include <stdbool.h>
#include <string.h>

static char* input = NULL;
int main(int argc,char** argv){
  printf("Hello, world!\n");

  do {
    printf("lisp>");
    size_t s;
    ssize_t input_size = getline(&input, &s, stdin);
    printf("%s",input);

  } while(strcmp(input,"exit\n") !=0) ;
}


