#ifndef QUEUE_H
#define QUEUE_H

#include "type.h"

void enqueue(PROC** queue, PROC* p);
PROC* dequeue(PROC** queue);

void printQ(PROC* p);
void printSleepList(PROC* p);
void printList(char* name, PROC *p);

#endif // QUEUE_H
