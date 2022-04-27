#include "ucode.c"

int main(int argc, char* argv[])
{
    for (int i = 1; i < argc; i++) {
        const char* filename = argv[i];
        STAT st;
        int r = stat(filename, &st);
        if (r >= 0) {
            unlink(filename);
        }
    }
    return 0;
}
