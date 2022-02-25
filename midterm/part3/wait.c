// wait.c file: ksleep(), kwakeup(), kexit()

extern PROC *running;
extern PROC *readyQueue;
extern PROC *sleepList;

int ksleep(int event)
{
    int sr = int_off();

    printf("P%d sleeping: event=%x\n", running->pid, event);

    running->event = event;
    running->status = SLEEP;

    enqueue(&sleepList, running);

    tswitch();

    int_on(sr);
}

int kwakeup(int event)
{
    int sr = int_off();

    PROC* nSleepList = 0;
    PROC* p;

    while (p = dequeue(&sleepList)) {
        if (p->status == SLEEP && p->event == event) {
            p->status = READY;
            enqueue(&readyQueue, p);
        } else {
            enqueue(&nSleepList, p);
        }
    }

    sleepList = nSleepList;

    int_on(sr);
}

int kexit(int exitCode)
{
    running->exitCode = exitCode;
    running->status = ZOMBIE;
    running->priority = 0;

    if (exitCode == 1) {
        kpipe->nreader--;
    } else if (exitCode == 2) {
        kpipe->nwriter--;
    }
    kwakeup((int)running->parent);
    tswitch();
}
