#include "ucode.c"

static const int NUM_COLS = 80;
static const int NUM_ROWS = 20;

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
        char* lp = line;
        int nLines = 0;
        int nLinesRequested = 0;
        int n;
        while (n = read(fd, buf, 1024)) {
            for (int i = 0; i < n; i++) {
                char c = buf[i];
                if (c == '\r' || c == '\n') {
                    if (nLinesRequested == 0) {
                        switch (getc()) {
                            case 'q':
                                return 0;
                            case '\r': case'\n':
                                nLinesRequested++;
                                break;
                            case ' ':
                                nLinesRequested += NUM_ROWS;
                                break;
                        }
                    }
                    printf("%s\n", line);
                    nLines++;
                    nLinesRequested--;

                    bzero(line, 512);
                    lp = line;
                } else {
                    *lp++ = c;
                }
            }
        }
        close(fd);
    } else { // more stdin
        char lc = '\0';
        char c;
        while (read(STDIN, &c, 1)) {
            total++;
            if (c == '\r' && lc != '\n' && lc != '\r') {
                putc('\n');
                putc('\r');
            } else {
                putc(c);
            }
        }
        lc = c;
    }
    return !total;
}
