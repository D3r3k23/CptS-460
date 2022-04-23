/************** test.c file ***************/

#include "ucode.c"

int main(int argc, char *argv[ ])
{
  int i;
  
  printf("this is a test\n");

  for (i=0; i<argc; i++){
    printf("argv[%d] = %s\n", i, argv[i]);
  }

  printf("end of test\n");

  exit(0);
}
