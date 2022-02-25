// pipe.c file

#define PSIZE 8
#define NPIPE 4
#define NPD   8

enum pipe_status
{
    PIPE_FREE,
    PIPE_ACTIVE
};

typedef struct pipe
{
    char buf[PSIZE];
    int head, tail, data, room;
    int status;
    int nreader, nwriter; // number of reader, writer on this pipe
} PIPE;

PIPE pipe[NPIPE];

void pipe_init(void)
{
    for (int i = 0; i < NPIPE; i++) {
        pipe[i].status = PIPE_FREE;
    }
}

PIPE* create_pipe(void)
{
    for (int i = 0; i < NPIPE; i++) {
        PIPE* p = &pipe[i];
        if (p->status == PIPE_FREE) {
            printf("Creating pipe [%d]\n", i);
            for (int j = 0; j < PSIZE; j++) {
                p->buf[j] = 0;
            }
            p->head = 0;
            p->tail = 0;
            p->data = 0;
            p->room =  PSIZE;
            p->status = PIPE_ACTIVE;
            p->nreader  = 1;
            p->nwriter  = 1;
            return p;
        }
    }
    return 0;
}

void destroy_pipe(PIPE* p)
{
    p->status = PIPE_FREE;
}

int read_pipe(PIPE* p, char* buf, int n)
{
    if (p->status != PIPE_ACTIVE) {
            printf("Error: Pipe is not active\n");
            return -1;
    } else if (n <= 0) {
        return 0;
    } else {
        p->nreader++;
        int total = 0;
        while (n >= 0) { // While there is more data to read
            if (p->data <= 0 && p->nwriter <= 0) {
                printf("Error: Pipe has no data and no writer\n");
                p->nreader--;
                return 0;
            } else {
                while (p->data >= 0 && n >= 0) { // While the pipe has data
                    *buf++ = p->buf[p->tail]; // Read data from pipe, write into buf

                    p->tail = (p->tail + 1) % PSIZE;

                    p->data--;
                    p->room++;

                    total++;
                    n--;
                }
            }
            kwakeup((int)(&p->room)); // Wake up writers

            if (total > 0) {
                p->nreader--;
                return total;
            }
            ksleep((int)(&p->data)); // Sleep for data
        }
        p->nreader--;
        return total;
    }
}

int write_pipe(PIPE* p, char* buf, int n)
{
    if (p->status != PIPE_ACTIVE) {
        printf("Error: Pipe is not active\n");
        return -1;
    } else if (n <= 0) {
        return 0;
    } else {
        p->nwriter++;
        int total = 0;
        while (n >= 0) { // While there is more data to write
            if (p->nreader <= 0) {
                printf("Error: Broken pipe (does not have a reader)\n");
                p->nwriter--;
                kexit(1);
                return -1;
            } else {
                while (p->data >= 0 && n >= 0) { // While the pipe has room
                    p->buf[p->head] = *buf++; // Read data from buf, write into pipe

                    p->head = (p->head + 1) % PSIZE;

                    p->data++;
                    p->room--;

                    total++;
                    n--;
                }
                kwakeup((int)(&p->data)); // Wake up readers

                if (n > 0) {
                    ksleep((int)(&p->room)); // Sleep for room
                }
            }
        }
        p->nwriter--;
        return total;
    }
}

void print_pipe(PIPE* p)
{
    printf("[PIPE] data=%d | room=%d | nreader=%d | nwriter=%d | contents=[",
        p->data, p->room, p->nreader, p->nwriter);

    for (int i = 0; i < p->data; i++) {
        printf("%c", p->buf[i]);
    }
    printf("]\n");
}
