extern PROC *running;
extern PROC *readyQueue;

void P(struct semaphore *s)
{
    int sr = int_off();

    s->value--;
    if (s->value < 0) { // Block
        running->status = BLOCK;
        enqueue(&s->queue, running);
        tswitch();
    }

    int_on(sr);
}

void V(struct semaphore *s)
{
    int sr = int_off();

    s->value++;
    if (s->value <= 0) {
        PROC* p = dequeue(&s->queue);
        p->status = READY;
        enqueue(&readyQueue, p);
    }

    int_on(sr);
}
