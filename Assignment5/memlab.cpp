//Group 1
//OS Assignment 5
//19CS10031 - Gandhi Abhishek Rajesh
//19CS10051 - Sajal Chhamunya

#include "memlab.h"
using namespace std;

mem *memPtr;
symbolTable* symTable;
markArray *mark;

mem::mem(int size):size(size){
    memBase = (int *)malloc(size*4);
    freeSize = size-1;
    int temp = size<<1;
    *memBase = temp;
    numBlocks = 1;
    int sem_job = pthread_mutex_init(&(mutex),NULL);
    if (sem_job < 0){
        perror("Could not create semaphore\n");
        exit(1);
    }
}

symbolTable::symbolTable():idx(0){
    stacks = new symbol[MAX_SYMBOL_TABLE_SIZE];
}

markArray::markArray():idx(0), arr(new int[MAX_SYMBOL_TABLE_SIZE]){
    int sem_job = pthread_mutex_init(&(mutex),NULL);
    if (sem_job < 0){
        perror("Could not create semaphore\n");
        exit(1);
    }
}

Ptr::Ptr(int idx,Type type) : idx(idx), type(type), size(4), isArray(0){}
Ptr::Ptr(int idx,Type type,int size,int isArray, int num_elements) : idx(idx), type(type), size(size), isArray(isArray), num_elements(num_elements) {}

int getSize(Type type) {
    if(type == INT) return 4;
    if(type == MEDIUM_INT) return 4;
    if(type == BOOLEAN) return 1;
    if(type == CHAR) return 1;
    return 1;
}

string getVarS(Type type){
    if(type==0) return "INT";
    if(type==1) return "CHAR";
    if(type==2) return "MEDIUM_INT";
    return "BOOLEAN";
}

pthread_t mythread;
void createMem(int size){
    //cout<<"Allocating intial memory"<<endl;
    ios_base::sync_with_stdio(false); 
    cin.tie(NULL);                    
    cout.tie(NULL);                    
    cout<<fixed;
    memPtr = new mem(size); // total size allocated is 4*size
    symTable = new symbolTable();
    pthread_attr_t attr; // will store attributes of the thread (e.g., stack size)
    pthread_attr_init (&attr); // get default attributes
    gc_initialize();
    pthread_create(&mythread, &attr, garbageCollector, NULL);
}

Ptr createVar(Type varType){
    cout<<"Allocating memory for varType :"<<getVarS(varType)<<endl;
    //cout<<"Assigning memory for a variable"<<endl;
    //int idx  = 0;
    int sizeRequired = 4;
    int *alpha = (memPtr->memBase);
    int temp = 0;
    pthread_mutex_lock(&memPtr->mutex);
    while(temp<memPtr->size){
        if((*(alpha+temp))%2==0 && ((*(alpha+temp))>>1)-1>=sizeRequired/4){
            cout<<"Adding new variable in symbol table at idx: "<<symTable->idx<<endl;
            symTable->stacks[symTable->idx].offset = temp+1;
            symTable->stacks[symTable->idx].status = 0;
            Ptr p(symTable->idx++,varType);

            if(((*(alpha+temp))>>1)-2-sizeRequired/4>0){
                (*(alpha+temp+1+sizeRequired/4)) = (((*(alpha+temp))>>1)-1-sizeRequired/4)<<1;
                memPtr->freeSize--;
                memPtr->numBlocks++;
            }
            *(alpha+temp) = ((1+sizeRequired/4)<<1) + 1;
            memPtr->freeSize -= (sizeRequired/4);
            pthread_mutex_unlock(&memPtr->mutex);
            //cout<<"Free location found"<<endl;
            return p;
        }
        temp += ((*(alpha+temp))>>1);

    }
    pthread_mutex_unlock(&memPtr->mutex);
    cout<<"\nNo free space available with size greater than required size"<<endl;
    exit(1);
    
}

void assignVar(Ptr &p,int val){
    
    if(p.type!=INT && p.type!=MEDIUM_INT)
        throw "Invalid type";
    
    int *alpha = (memPtr->memBase);
    cout<<"Assigning int or medium int variable at logical address: "<<p.idx<<" = "<<val<<endl;
    *(alpha+symTable->stacks[p.idx].offset) = val;
    //pthread_mutex_unlock(&memPtr->mutex);
}

void assignVar(Ptr &p,bool val){
    if(p.type!=BOOLEAN)
        throw "Invalid type";
    int *alpha = (memPtr->memBase);
    cout<<"Assigning bool type variable at logical address: "<<p.idx<<" = "<<val<<endl;
    *(alpha+symTable->stacks[p.idx].offset) = val;
}

void assignVar(Ptr &p,char val){
    if(p.type!=CHAR)
        throw "Invalid type";
    int *alpha = (memPtr->memBase);
    cout<<"Assigning char type variable at logical address: "<<p.idx<<" = "<<val<<endl;
    *(alpha+symTable->stacks[p.idx].offset) = val;
}

void getVar(Ptr &p,int &val){
    if(p.type!=INT && p.type!=MEDIUM_INT)
        throw "Invalid type";
    int *alpha = (memPtr->memBase);
    cout<<"Extracting int or medium int variable at logical address: "<<p.idx<<endl;
    val = *(alpha+symTable->stacks[p.idx].offset);
}

void getVar(Ptr &p,bool& val){
    if(p.type!=BOOLEAN)
        throw "Invalid type";
    int *alpha = (memPtr->memBase);
    cout<<"Extracting bool type variable at logical address: "<<p.idx<<endl;
    val = *(alpha+symTable->stacks[p.idx].offset);
}

void getVar(Ptr &p,char& val){
    if(p.type!=CHAR)
        throw "Invalid type";
    int *alpha = (memPtr->memBase);
    cout<<"Extracting char type variable at logical address: "<<p.idx<<endl;
    val = *(alpha+symTable->stacks[p.idx].offset);
}

Ptr createArr(int size, Type varType){
    cout<<"Allocating memory for array of varType :"<<getVarS(varType)<<" and size = "<<size<<endl;
    int sizeRequired = size*getSize(varType);
    if(sizeRequired%4){
        sizeRequired = (sizeRequired/4)*4+4;
    }
    int *alpha = (memPtr->memBase);
    int temp = 0;
    pthread_mutex_lock(&memPtr->mutex);
    while(temp<memPtr->size){
        if((*(alpha+temp))%2==0 && ((*(alpha+temp))>>1)-1>=sizeRequired/4){
            cout<<"Adding new variable in symbol table at idx: "<<symTable->idx<<endl;
            symTable->stacks[symTable->idx].offset = temp+1;
            symTable->stacks[symTable->idx].status = 0;
            Ptr p(symTable->idx++,varType,sizeRequired,1,size);
            if(((*(alpha+temp))>>1)-2-sizeRequired/4>0){
                (*(alpha+temp+sizeRequired/4+1)) = (((*(alpha+temp))>>1)-1-sizeRequired/4)<<1;
                memPtr->freeSize--;
                memPtr->numBlocks++;
            }
            *(alpha+temp) = ((1+sizeRequired/4)<<1)+1;
            memPtr->freeSize -= sizeRequired/4;
            pthread_mutex_unlock(&memPtr->mutex);
            //cout<<"Free location found"<<endl;
            return p;
        }
        temp += ((*(alpha+temp))>>1);
    }
    pthread_mutex_unlock(&memPtr->mutex);
    cout<<"No free space available with size greater than required size"<<endl;
    exit(1);
}

void assignArr(Ptr &p,int a[],int n){
    if(p.type!=INT && p.type!=MEDIUM_INT)
        throw "Invalid type";
    int *alpha = (memPtr->memBase + symTable->stacks[p.idx].offset);
    cout<<"Assigning int or medium int array at logical address: "<<p.idx<<endl;
    for(int i=0;i<n;i++){
        *alpha = a[i];
        alpha++;
    }
    //pthread_mutex_unlock(&memPtr->mutex);
}
void assignArr(Ptr &p,char a[],int n){
    if(p.type!=CHAR)
        throw "Invalid type";
    char *alpha = (char *)(memPtr->memBase + symTable->stacks[p.idx].offset);
    cout<<"Assigning char array at logical address: "<<p.idx<<endl;
    for(int i=0;i<n;i++){
        *alpha = a[i];
        alpha++;
    }
}
void assignArr(Ptr &p,bool a[],int n){
    if(p.type!=BOOLEAN)
        throw "Invalid type";
    bool *alpha =(bool *)(memPtr->memBase + symTable->stacks[p.idx].offset);
    cout<<"Assigning bool array at logical address: "<<p.idx<<endl;
    for(int i=0;i<n;i++){
        *alpha = a[i];
        alpha++;
    }
}

void assignArr(Ptr &p,int idx,int val){
    if(p.type!=INT && p.type!=MEDIUM_INT)
        throw "Invalid type";
    cout<<"Assigning int or medium int array element at logical address "<<p.idx<<" and at index "<<idx<<endl;
    int *alpha = (memPtr->memBase + symTable->stacks[p.idx].offset);
    alpha +=idx;
    *alpha = val;
    //pthread_mutex_unlock(&memPtr->mutex);
}

void assignArr(Ptr &p,int idx,char val){
    if(p.type!=CHAR)
        throw "Invalid type";
    cout<<"Assigning char array element at logical address "<<p.idx<<" and at index "<<idx<<endl;
    char *alpha = (char *)(memPtr->memBase + symTable->stacks[p.idx].offset);
    alpha +=idx;
    *alpha = val;
}

void assignArr(Ptr &p,int idx,bool val){
    if(p.type!=BOOLEAN)
        throw "Invalid type";
    cout<<"Assigning bool array element at logical address "<<p.idx<<" and at index "<<idx<<endl;
    bool *alpha =(bool *)(memPtr->memBase + symTable->stacks[p.idx].offset);
    alpha +=idx;
    *alpha = val;
}


void freeElem(Ptr &p){
    cout<<"freeing memory with stack idx: "<<p.idx<<endl;
    int *alpha = memPtr->memBase + symTable->stacks[p.idx].offset;
    alpha--;
    *alpha = ((*alpha)>>1)<<1;
    memPtr->freeSize += (((*alpha)>>1)-1);
    symTable->stacks[p.idx].status = 1;

}
void freeElem(int physical_offset){
    cout<<"freeing memory with physical_offset: "<<physical_offset<<endl;
    int *alpha = memPtr->memBase + physical_offset;
    alpha--;
    *alpha = ((*alpha)>>1)<<1;
    memPtr->freeSize += (((*alpha)>>1)-1);
}

void freeMem(){
    cout<<"Terminating the program"<<endl;
    pthread_kill(mythread,SIGINT);
    free(memPtr);
    free(symTable);
    free(mark);
}

void startScope(){
    cout<<"new scope started"<<endl;
    symTable->stacks[symTable->idx++].offset = -1;
}

void endScope(){
    cout<<"end scope, empty the stack till -1 and mark them"<<endl;
    for(int i = symTable->idx-1;i>=0;i--){
        if(symTable->stacks[i].offset == -1){
            symTable->idx = i;
            break;
        }
        pthread_mutex_lock(&mark->mutex);
        if(mark->idx<MAX_SYMBOL_TABLE_SIZE){
                mark->arr[mark->idx++] = symTable->stacks[i].offset;
        }
        else{
            symTable->idx = i;
            break;
        }
        pthread_mutex_unlock(&mark->mutex);
    }
}

void gc_initialize(){
    cout<<"initializing garbageCollector"<<endl;
    mark = new markArray();
}

void compactMem(){
    cout<<"Iterating through memory and compressing it"<<endl;
    pthread_mutex_lock(&memPtr->mutex);
    int temp = 0;
    int *alpha = (memPtr->memBase);
    bool update = false;
    int updateLoc = 0;
    while(temp<memPtr->size){
        if((*(alpha+temp))%2==0){
            if(update){
                (*(alpha+updateLoc)) = ((((*(alpha+updateLoc))>>1)+((*(alpha+temp))>>1))<<1);
                memPtr->numBlocks--;
            }
            else{
                update = true;
                updateLoc = temp;
            }
        }
        else{
            update = false;
        }
        temp += ((*(alpha+temp))>>1);
    }
    pthread_mutex_unlock(&memPtr->mutex);
}

void gc_run(){
    bool compress = false;
    pthread_mutex_lock(&mark->mutex);
    
    if(mark->idx!=0){
        cout<<"Removing marked elements"<<endl;
        compress = true;
        for(int i=mark->idx-1;i>=0;i--)
            freeElem(mark->arr[i]);  
        mark->idx = 0;
    }
    pthread_mutex_unlock(&mark->mutex);

    if(compress && memPtr->numBlocks>memPtr->size/AVERAGE_FREE_BLOCK_SIZE){
        compactMem();
    }
}

void *garbageCollector(void *){
    //gc_initialize();
    while(1){
        gc_run();
        usleep(1000*GC_SLEEP_TIME_MS);
    }
}  

void getArr(Ptr &p,int a[],int n){
    if(p.type!=INT && p.type!=MEDIUM_INT)
        throw "Invalid type";
    if(p.num_elements!=n)
        throw "Invalid size";
    cout<<"Extracting int or medium int array from logical address: "<<p.idx<<endl;
    int *alpha = memPtr->memBase+symTable->stacks[p.idx].offset;
    for(int i=0;i<n;i++){
        a[i] = *alpha;
        alpha++;
    }
    
}

void getArr(Ptr &p,char a[],int n){
    if(p.type!=CHAR)
        throw "Invalid type";
    if(p.num_elements!=n)
        throw "Invalid size";

    cout<<"Extracting char array from logical address: "<<p.idx<<endl;

    char *alpha = (char *)(memPtr->memBase+symTable->stacks[p.idx].offset);
    for(int i=0;i<n;i++){
        a[i] = *alpha;
        alpha++;
    }
    
}

void getArr(Ptr &p,bool a[],int n){
    if(p.type!=BOOLEAN)
        throw "Invalid type";
    if(p.num_elements!=n)
        throw "Invalid size";
    cout<<"Extracting bool array from logical address: "<<p.idx<<endl;
    bool *alpha = (bool *)(memPtr->memBase+symTable->stacks[p.idx].offset);
    for(int i=0;i<n;i++){
        a[i] = *alpha;
        alpha++;
    }
    
}

void getArr(Ptr &p,int idx,int &val){
    if(p.type!=INT && p.type!=MEDIUM_INT)
        throw "Invalid type";
    cout<<"Extracting int or medium_int array element from logical address "<<p.idx<<" and the idx is "<<idx<<endl;
    int *alpha = memPtr->memBase+symTable->stacks[p.idx].offset;
    alpha+=idx;
    val = *alpha;

}

void getArr(Ptr &p,int idx,char &val){
    if(p.type!=CHAR)
        throw "Invalid type";
    cout<<"Extracting char array from logical address: "<<p.idx<<" and the idx is "<<idx<<endl;

    char *alpha = (char *)(memPtr->memBase+symTable->stacks[p.idx].offset);
    alpha+=idx;
    val = (*alpha);
}

void getArr(Ptr &p,int idx,bool &val){
    if(p.type!=BOOLEAN)
        throw "Invalid type";
    cout<<"Extracting bool array from logical address "<<p.idx<<" and the idx is "<<idx<<endl;

    bool *alpha = (bool *)(memPtr->memBase+symTable->stacks[p.idx].offset);
    alpha+=idx;
    val = (*alpha);
}

/**
 * Ptr a = createVar(INT);
 * a.id -> symboltable[a.id] = memory location
 * 
 */