#include "ucode.c"

int main(int argc, char* argv[])
{
    printf("This is a Test\n");

    for (int i = 0; i < argc; i++) {
        printf("arg[%d] = %s\n", i, argv[i]);
    }

    printf("Exiting Test\n");
    exit(0);
}
