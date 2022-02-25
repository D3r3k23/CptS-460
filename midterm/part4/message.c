/******** message.c file ************/

// READ Chapter 5.13.4.3; Use its code

struct semaphore nmbuf;
struct semaphore mlock;

#define NMBUF 10

MBUF mbuf[NMBUF];
MBUF* mbufList = 0;

void menqueue(MBUF** queue, MBUF* p)
{
    p->next = 0;
    if (!(*queue)) { // Empty
        *queue = p;
    } else {
        MBUF* q = *queue;
        while (q->next) { // Find last MBUF
            q = q->next;
        }
        q->next = p;
    }
}

MBUF* mdequeue(MBUF** queue)
{
    MBUF* p = *queue;
    if (p) {
        *queue = p->next;
    }
    return p;
}

int msg_init()
{
    for (int i = 0; i < NMBUF; i++) {
        menqueue(&mbufList, &mbuf[i]);
    }

    nmbuf.value = NMBUF;
    nmbuf.queue = 0;

    mlock.value = 1;
    mlock.queue = 0;
}

MBUF* get_mbuf()
{
    P(&nmbuf);
    P(&mlock);

    MBUF* mp = mdequeue(&mbufList);

    V(&mlock);
    return mp;
}

int put_mbuf(MBUF* mp)
{
    P(&mlock);

    menqueue(&mbufList, mp);

    V(&mlock);
    V(&nmbuf);
}

void send(char* msg, int pid)
{
    if (pid > 0) {
        PROC* p = &proc[pid];
        MBUF* mp = get_mbuf();

        mp->pid = running->pid;
        strcpy(mp->contents, msg);

        P(&p->mlock);

        menqueue(&p->mqueue, mp);

        V(&p->mlock);
        V(&p->nmsg);
    }
}

int recv(char* msg)
{
    P(&running->nmsg);
    P(&running->mlock);

    MBUF* mp = mdequeue(&running->mqueue);

    V(&running->mlock);

    strcpy(msg, mp->contents);
    int sender = mp->pid;
    put_mbuf(mp);

    return sender;
}
