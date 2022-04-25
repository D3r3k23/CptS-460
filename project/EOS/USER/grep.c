#include "ucode.c"

int grep_line(const char* pattern, const char* line);

int main(int argc, char* argv[])
{
    if (argc < 2) {
        printf("Missing argument: pattern\n");
        return -1;
    }
    const char* pattern = argv[1];
    int matches = 0;

    if (argc >= 3) { // grep file
        const char* filename = argv[2];
        int fd = open(filename, O_RDONLY);
        if (fd == -1) {
            printf("grep error: could not open %s\n", filename);
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
                    if (grep_line(pattern, line)) {
                        printf("%s\n", line);
                        matches++;
                    }
                    bzero(line, 512);
                    lp = line;
                } else {
                    *lp++ = c;
                }
            }
        }
        close(fd);
        return matches;
    }
}

int grep_line(const char* pattern, const char* line)
{
    for (int i = 0; i < strlen(line); i++) {
        for (int j = 0; j < strlen(pattern); j++) {
            if (i + j >= strlen(line)) {
                break;
            } else if (pattern[j] != line[i + j]) {
                break;
            } else if (j >= strlen(pattern) - 1) { // Entire pattern found in line
                return 1;
            }
        }
    }
    return 0;
}
