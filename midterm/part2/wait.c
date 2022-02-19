#include "wait.h"

#include "t.h"
#include "queue.h"
#include "vid.h"

extern int int_off(void);
extern void int_on(int);

int ksleep(int event)
{
    int sr = int_off();

    running->event = event;
    running->status = SLEEP;
    enqueue(&sleepList, running);

    tswitch();

    int_on(sr);
}

int kwakeup(int event)
{
    PROC* p;
    PROC* nSleepList = 0;

    int sr = int_off();

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

int kexit(int exitValue)
{
    PROC* child;
    PROC* nextChild;
    PROC* temp;
    PROC* p1 = &proc[1];

    kprintf("proc %d in kexit(), value=%d\n", running->pid, exitValue);

    if (running->child) {
        child = running->child;
        running->child = 0;

        while (child) {
            // Move orphaned proc to P1's child list
            if (!p1->child) {
                p1->child = child;
            } else {
                temp = p1->child;
                while (temp->sibling) // Find last child
                    temp = temp->sibling;

                temp->sibling = child;
            }
            nextChild = child->sibling;
            child->sibling = 0;
            child = nextChild;
        }
    }

    running->exitCode = exitValue;
    running->status = ZOMBIE;
    kwakeup((int)running->parent);
    tswitch();
}

int kwait(int* status) // This status is NOT PROC.status
{
    PROC* p;
    int exitCode;
    int zid;

    if (!running->child) {
        kprintf("Running proc has no child\n");
        return -1;
    }

    p = running->child;
    if (p->status == ZOMBIE) { // First child is zombie
        exitCode = p->exitCode;
        zid = p->pid;
        p->status = FREE;
        enqueue(&freeList, p);
        running->child = p->sibling;
    } else { // Search siblings
        while (p->sibling) {
            if (p->sibling->status == ZOMBIE) {
                exitCode = p->sibling->exitCode;
                zid = p->sibling->pid;
                p->status = FREE;
                enqueue(&freeList, p->sibling);
                p->sibling = p->sibling->sibling;
            }
            ksleep((int)running);

            p = p->sibling;
        }
    }
    if (status) {
        *status = exitCode;
    }
    return zid;
}
