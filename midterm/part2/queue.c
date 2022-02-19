#include "queue.h"

#include "vid.h"

void enqueue(PROC** queue, PROC* p)
{
    PROC* q  = *queue;

    if (!q) {
        *queue = p;
        p->next = 0;
    } else if ((*queue)->priority < p->priority){
        p->next = *queue;
        *queue = p;
    } else {
        while (q->next && p->priority <= q->next->priority) {
            q = q->next;
        }
        p->next = q->next;
        q->next = p;
    }
}

PROC* dequeue(PROC** queue)
{
    PROC *p = *queue;
    if (p)
        *queue = p->next;
    return p;
}

void printQ(PROC *p)
{
    kprintf("readyQueue = ");
    while(p) {
        kprintf("[%d%d]->", p->pid,p->priority);
        p = p->next;
    }
    kprintf("NULL\n");
}

void printSleepList(PROC* p)
{
    kprintf("sleepList   = ");
    while(p) {
        kprintf("[%devent=%d]->", p->pid,p->event);
        p = p->next;
    }
    kprintf("NULL\n");
}

void printList(char* name, PROC* p)
{
    kprintf("%s = ", name);
    while (p) {
        kprintf("[%d%d]->", p->pid, p->priority);
        p = p->next;
    }
    kprintf("NULL\n");
}

