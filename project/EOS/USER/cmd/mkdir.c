#include "ucode.c"

int main(int argc, char* argv[])
{
    if (argc < 2) {
        printf("Enter a directory name\n");
        return 2;
    } else {
        for (int i = 1; i < argc; i++) {
            const char* dirname = argv[i];
            STAT st;
            int r = stat(dirname, &st);
            if (r >= 0) {
                printf("Error: %s already exists\n", dirname);
                return 1;
            } else {
                mkdir(dirname);
                return 0;
            }
        }
    }
}
