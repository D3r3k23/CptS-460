int ksleep(int event)
{
    int sr = int_off();
    printf("P%d ksleep: event=%x\n", running->pid, event);

    running->status = SLEEP;
    running->event = event;

    enqueue(&sleepList, running);
    printSleepList(sleepList);

    int_on(sr);
    tswitch();

    return 0;
}

int kwakeup(int event)
{
    int sr = int_off();
    printf("P%d kwakeup: event=%x\n", running->pid, event);

    PROC* nSleepList = 0;

    PROC* p;
    while (p = dequeue(&sleepList)) {
        if (p->event == event) {
            printf("Wake up P%d\n", p->pid);
            p->status = READY;
            enqueue(&readyQueue, p);
        } else {
            enqueue(&nSleepList, p);
        }
    }
    sleepList = nSleepList;

    int_on(sr);

    return 0;
}

int find_child_proc(PROC* p)
{
    for (int i = 1; i < NPROC; i++) {
        PROC* cp = &proc[i];
        if (cp->status != FREE && cp->ppid == p->pid) {
            printf("Found child P%d to parent P%d\n", cp->pid, p->pid);
            return cp;
        }
    }
    return 0;
}

PROC* find_zombie_child(PROC* p)
{
    for (int i = 1; i < NPROC; i++) {
        PROC* cp = &proc[i];
        if (cp->status == ZOMBIE && cp->ppid == p->pid) {
            printf("Found zombie child P%d to parent P%d\n", cp->pid, p->pid);
            return cp;
        }
    }
    return 0;
}

int kexit(int value)
{
    printf("P%d kexit: value=%d\n", value);

    if (running->pid == 1) {
        return -1;
    } else {
        int wake_p1 = !!find_child_proc(running);

        PROC* p;
        while (p = find_child_proc(running)) {
            printf("Send orphaned child P%d to P1\n", p->pid);
            p->ppid = 1;
            p->parent = &proc[1];
        }
        running->status = ZOMBIE;
        running->exitCode = value;

        kwakeup((int)running->parent);
        if (wake_p1)
            kwakeup((int)&proc[1]);

        tswitch();
        return 0;
    }
}

int kwait(int* status)
{
    printf("P%d kwait\n", running->pid);

    if (!find_child_proc(running)) {
        printf("P%d child error\n", running->pid);
        return -1;
    } else {
        while (1) {
            PROC* p;
            if (p = find_zombie_child(running)) {
                if (status)
                    *status = p->exitCode;
                p->status = FREE;
                putproc(p);
                return p->pid;
            } else {
                printf("P%d sleeping\n", running->pid);
                ksleep((int)running);
            }
        }
    }
}
