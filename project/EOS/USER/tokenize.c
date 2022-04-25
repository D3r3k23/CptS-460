#ifndef TOKEN_LEN
#define TOKEN_LEN 32
#endif

// Tokenize string into tokens by delim
// tokens: char buf[MAX][TOKEN_LEN]
// max: Max number of tokens
int tokenize(const char* string, char delim, char (*tokens)[TOKEN_LEN], int max)
{
    int n = 0;
    char* tp = tokens[0];
    char* cp = string;

    while (1) {
        if (n >= max) {
            return max;
        } else if (*cp == '\0') {
            n++;
            *tp = '\0';
            return n;
        } else if (*cp == delim) {
            n++;
            *tp = '\0';
            tp = tokens[n];
            cp++;
        } else {
            *tp++ = *cp++;
        }
    }
}
