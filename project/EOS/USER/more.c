#include "ucode.c"

static const int NUM_COLS = 80;

int main(int argc, char* argv[])
{
    int total = 0;
    if (argc >= 2) { // more file
        const char* filename = argv[1];
        int fd = open(filename, O_RDONLY);
        if (fd == -1) {
            printf("more error: could not open %s\n", filename);
            return -1;
        }
        char buf[1024];
        char line[512];
        bzero(line, 512);
        char* lp = line;
        int n;
        while (n = read(fd, buf, 1024)) {
            for (int i = 0; i < n; i++) {
                char c = buf[i];
                if (c == '\r' || c == '\n') {
                    switch (getc())
                    {
                        case 'q': return 0;
                        default:
                            printf("%s\n", line);
                    }
                    bzero(line, 512);
                    lp = line;
                } else {
                    *lp++ = c;
                }
            }
        }
        close(fd);
    } else { // more stdin
        printf("Still need to add more stdin\n");
    }
    return total;
}
