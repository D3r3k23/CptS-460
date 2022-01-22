/*********  t.c file *********************/

extern char getc(void);
extern void putc(char c);

void prints(char* s)
{
    while (*s) {
        putc(*s++);
    }
}

void gets(char* s)
{
    while ((*s = getc()) != '\r') {
        putc(*s++);
    }
    *s = '\0';
}

char ans[64];

int main()
{
    while (1) {
        prints("What's your name? ");
        gets(ans);
        prints("\n\r");

        if (ans[0] == '\0') {
            prints("return to assembly and hang\n\r");
            break;
        }
        prints("Welcome "); prints(ans); prints("\n\r");
    }
    return 0;
}
