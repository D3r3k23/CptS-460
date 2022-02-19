#ifndef WAIT_H
#define WAIT_H

int tswitch();

int ksleep(int event);
int kwakeup(int event);
int kexit(int exitValue);
int kwait(int* status);

#endif // WAIT_H
