#ifndef MEM_LAB_H
#define MEM_LAB_H


#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/shm.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <pthread.h>

void * createMem(int size);
void createVar();
void assignVar();
void createArr();
void assignArr();
void freeElem();


#endif 