#include "../type.h"

int body(), goUmode();

int kfe(char* cmd_line)
{
    kprintf("kfe: %s\n", cmd_line);

    PROC* p = getproc();
    if (!p) {
        kprintf("kfe failed\n");
        return -1;
    }

    /* initialize the new PROC and its kstack */
    p->status = READY;
    p->ppid = running->pid;
    p->parent = running;
    p->priority = 128;
    p->inkmode = 1;
    p->time = 0;
    p->cpu = 0;
    p->type = PROCESS;
    p->cpsr = 0x10;

    p->res->size = running->res->size;
    p->res->uid = running->res->uid;
    p->res->gid = running->res->gid;
    p->res->cwd = running->res->cwd;
    p->tcount = 1;
    p->res->cwd->refCount++;
    strcpy(p->res->tty, running->res->tty);

    // p->res->signal, p->res->sig[] are cleared in kexit()
    p->res->signal = 0;
    for (int i = 0; i < NSIG; i++) {
        p->res->sig[i] = 0;
    }

    /***** clear message queue ******/
    p->res->mqueue = 0;
    p->res->mlock.value = 1; p->res->mlock.queue = 0;
    p->res->message.value = 0; p->res->message.queue = 0;

    /**** copy file descriptors ****/
    for (int i = 0; i < NFD; i++) {
        p->res->fd[i] = running->res->fd[i];
        if (p->res->fd[i] != 0) {
            p->res->fd[i]->refCount++;
        }
    }

    // set kstack to resume to body
    for (int i = 1; i < 29; i++) { // all 28 cells = 0
        p->kstack[SSIZE - i] = 0;
    }

    p->kstack[SSIZE - 1] = (int)running->upc;
    p->kstack[SSIZE - 15] = (int)goUmode;
    p->ksp = &(p->kstack[SSIZE - 28]);

    char kline[128];
    kstrcpy(kline, cmd_line);

    char filename[128];
    char* fp = filename;
    char* kp = kline;
    while (*kp && *kp != ' ') {
        *fp++ = *kp++;
    }
    *fp = '\0';

    u32 BA = p->res->pgdir[2048] & 0xFFFFF000;
    kprintf("load file \"%s\" to %x\n", filename, BA);

    // load file to Umode image
    if (load(filename, p) <= 0 ){
        printf("kfe load error\n");
        return -1;
    }

    // copy cmd_line to high end of Ustack in Umode image
    u32 Btop = BA + 0x100000;
    u32 Busp = Btop - 32;

    char* cp = (char*)Busp;
    kstrcpy(cp, kline);

    p->usp = (int*)VA(0x100000 - 32);
    p->upc = (int*)VA();

    p->kstack[SSIZE - 14] = (int)VA(0x100000 - 32); // R0 to Umode
    p->kstack[SSIZE - 1] = (int)VA(0);              // ulr to Umode
    // printf("usp=%x contents=%s\n", p->usp, (char *)p->usp);
    strcpy(running->res->name, filename);

    enqueue(&readyQueue, p);
    printQ(readyQueue);

    return p->pid;
}
