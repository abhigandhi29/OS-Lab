//Group 1
//OS Assignment 5
//19CS10031 - Gandhi Abhishek Rajesh
//19CS10051 - Sajal Chhamunya

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
#include <string>
#define MAX_SYMBOL_TABLE_SIZE 1000
#define GC_SLEEP_TIME_MS 50
#define AVERAGE_FREE_BLOCK_SIZE 100000
using namespace std;


enum Type {
    INT,
    CHAR,
    MEDIUM_INT,
    BOOLEAN
};

int getSize(Type type);
struct mem{
    int *memBase;
    int size;
    int freeSize;
    int numBlocks;
    pthread_mutex_t mutex;
    mem(int size);
};
extern mem *memPtr;

struct symbol{
    int offset;
    bool status = 0;
};

struct markArray{
    int idx;
    int *arr;
    pthread_mutex_t mutex;
    markArray();
};
extern markArray *mark;
struct symbolTable{
    symbol *stacks;
    int idx;
    symbolTable();
};
extern symbolTable* symTable;

struct Ptr{
    int idx;
    Type type;
    int size;
    bool isArray;
    int num_elements;
    Ptr(int idx,Type type);
    Ptr(int idx,Type type,int size,int isArray,int num_elements);
};


void createMem(int size);
Ptr createVar(Type varType);
void assignVar(Ptr &,int var);
void assignVar(Ptr &,bool var);
void assignVar(Ptr &,char var);
void getVar(Ptr &,int& var);
void getVar(Ptr &,bool& var);
void getVar(Ptr &,char& var);
Ptr createArr(int size, Type varType);
void assignArr(Ptr &,int a[],int n);
void assignArr(Ptr &,char a[],int n);
void assignArr(Ptr &,bool a[],int n);
void assignArr(Ptr &,int idx,int val);
void assignArr(Ptr &,int idx,char val);
void assignArr(Ptr &,int idx,bool val);
void getArr(Ptr &,int a[],int n);
void getArr(Ptr &,char a[],int n);
void getArr(Ptr &,bool a[],int n);
void getArr(Ptr &,int idx,int &val);
void getArr(Ptr &,int idx,char &val);
void getArr(Ptr &,int idx,bool &val);
void freeElem(Ptr &);
void freeElem(int pysical_offset);
void *garbageCollector(void *);
void gc_initialize();
void gc_run();
void startScope();
void endScope();
void freeMem();
string getVarS(Type type);
void compactMem();



#endif 