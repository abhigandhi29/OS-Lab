#include "memlab.h"
using namespace std;

mem *memPtr;
symbolTable* symTable;
markArray *mark;

mem::mem(int size):size(size){
    memBase = (int *)malloc(4*size);
    int temp = size<<1;
    *memBase = temp;
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
pthread_t mythread;
void createMem(int size){
    cout<<"Allocating intial memory"<<endl;
    memPtr = new mem(size); // total size allocated is 4*size
    symTable = new symbolTable();
    pthread_attr_t attr; // will store attributes of the thread (e.g., stack size)
    pthread_attr_init (&attr); // get default attributes
    pthread_create(&mythread, &attr, garbageCollector, NULL);

}

Ptr createVar(Type varType){
    cout<<"Assigning memory for a variable"<<endl;
    int idx  = 0;
    int sizeRequired = 4;
    int *alpha = (memPtr->memBase);
    int temp = 0;
    pthread_mutex_lock(&memPtr->mutex);
    while(temp<memPtr->size){
        //cout<<temp<<endl;
        if((*(alpha+temp))%2==0 && ((*(alpha+temp))>>1)-1>=sizeRequired/4){
            symTable->stacks[symTable->idx].offset = temp+1;
            Ptr p(symTable->idx++,varType);
            //cout<<(((*(alpha+temp))>>1)-2)<<endl;
            if(((*(alpha+temp))>>1)-2-sizeRequired/4>0){
                cout<<"creating one more block in memory"<<endl;
                (*(alpha+temp+1+sizeRequired/4)) = (((*(alpha+temp))>>1)-1-sizeRequired/4)<<1;
            }
            *(alpha+temp) = ((1+sizeRequired/4)<<1) + 1;
            pthread_mutex_unlock(&memPtr->mutex);
            cout<<"Free location found"<<endl;
            return p;
        }
        temp += ((*(alpha+temp))>>1);

    }
    pthread_mutex_unlock(&memPtr->mutex);
    cout<<"No big free space available"<<endl;
    exit(1);
    // Ptr p(-1,varType);
    // return p; 
}

void assignVar(Ptr &p,int val){
    //cout<<"here"<<p.type<<endl;
    if(p.type!=INT && p.type!=MEDIUM_INT)
        throw "Invalid type";
    //cout<<"here"<<endl;
    int *alpha = (memPtr->memBase);
    //cout<<"here"<<endl;
    //pthread_mutex_lock(&memPtr->mutex);
    cout<<"Assigning variable in allocated memory"<<endl;
    *(alpha+symTable->stacks[p.idx].offset) = val;
    //pthread_mutex_unlock(&memPtr->mutex);
}

void assignVar(Ptr &p,bool val){
    if(p.type!=BOOLEAN)
        throw "Invalid type";
    int *alpha = (memPtr->memBase);
    //pthread_mutex_lock(&memPtr->mutex);
    cout<<"Assigning variable in allocated memory"<<endl;
    *(alpha+symTable->stacks[p.idx].offset) = val;
    //pthread_mutex_unlock(&memPtr->mutex);
}

void assignVar(Ptr &p,char val){
    if(p.type!=CHAR)
        throw "Invalid type";
    int *alpha = (memPtr->memBase);
    //pthread_mutex_unlock(&memPtr->mutex);
    cout<<"Assigning variable in allocated memory"<<endl;
    *(alpha+symTable->stacks[p.idx].offset) = val;
    //pthread_mutex_unlock(&memPtr->mutex);
}

void getVar(Ptr &p,int &val){
    //cout<<"here"<<p.type<<endl;
    if(p.type!=INT && p.type!=MEDIUM_INT)
        throw "Invalid type";
   // cout<<"here"<<endl;
    int *alpha = (memPtr->memBase);
    //cout<<"here"<<endl;
    //pthread_mutex_lock(&memPtr->mutex);
    cout<<"Extracting a variable value from memory"<<endl;
    val = *(alpha+symTable->stacks[p.idx].offset);
    //pthread_mutex_unlock(&memPtr->mutex);
}

void getVar(Ptr &p,bool& val){
    if(p.type!=BOOLEAN)
        throw "Invalid type";
    int *alpha = (memPtr->memBase);
    //pthread_mutex_lock(&memPtr->mutex);
    cout<<"Extracting a variable value from memory"<<endl;
    val = *(alpha+symTable->stacks[p.idx].offset);
    //pthread_mutex_unlock(&memPtr->mutex);
}

void getVar(Ptr &p,char& val){
    if(p.type!=CHAR)
        throw "Invalid type";
    int *alpha = (memPtr->memBase);
    //pthread_mutex_unlock(&memPtr->mutex);
    cout<<"Extracting a variable value from memory"<<endl;
    val = *(alpha+symTable->stacks[p.idx].offset);
    //pthread_mutex_unlock(&memPtr->mutex);
}

Ptr createArr(int size, Type varType){
    cout<<"Assigning memory for a array"<<endl;
    int sizeRequired = size*getSize(varType);
    if(sizeRequired%4){
        sizeRequired = (sizeRequired/4)*4+4;
    }
    int *alpha = (memPtr->memBase);
    int temp = 0;
    pthread_mutex_lock(&memPtr->mutex);
    while(temp<memPtr->size){
        if((*(alpha+temp))%2==0 && ((*(alpha+temp))>>1)-1>=sizeRequired/4){
            symTable->stacks[symTable->idx].offset = temp+1;
            Ptr p(symTable->idx++,varType,sizeRequired,1,size);
            if(((*(alpha+temp))>>1)-2-sizeRequired/4>0){
                (*(alpha+temp+sizeRequired/4+1)) = (((*(alpha+temp))>>1)-1-sizeRequired/4)<<1;
            }
            *(alpha+temp) = ((1+sizeRequired/4)<<1)+1;
            pthread_mutex_unlock(&memPtr->mutex);
            cout<<"Free location found"<<endl;
            return p;
        }
        temp += ((*(alpha+temp))>>1);
    }
    pthread_mutex_unlock(&memPtr->mutex);
    cout<<"No big free space available"<<endl;
    exit(1);
}

void assignArr(Ptr &p,int a[],int n){
    if(p.type!=INT && p.type!=MEDIUM_INT)
        throw "Invalid type";
    //pthread_mutex_lock(&memPtr->mutex);
    int *alpha = (memPtr->memBase + symTable->stacks[p.idx].offset);
    cout<<"Assigning array elements"<<endl;
    for(int i=0;i<n;i++){
        *alpha = a[i];
        alpha++;
    }
    //pthread_mutex_unlock(&memPtr->mutex);
}
void assignArr(Ptr &p,char a[],int n){
    if(p.type!=CHAR)
        throw "Invalid type";
    //pthread_mutex_lock(&memPtr->mutex);
    char *alpha = (char *)(memPtr->memBase + symTable->stacks[p.idx].offset);
    cout<<"Assigning array elements"<<endl;
    for(int i=0;i<n;i++){
        *alpha = a[i];
        alpha++;
    }
    //pthread_mutex_unlock(&memPtr->mutex);
}
void assignArr(Ptr &p,bool a[],int n){
    if(p.type!=BOOLEAN)
        throw "Invalid type";
    //pthread_mutex_lock(&memPtr->mutex);
    bool *alpha =(bool *)(memPtr->memBase + symTable->stacks[p.idx].offset);
    cout<<"Assigning array elements"<<endl;
    for(int i=0;i<n;i++){
        *alpha = a[i];
        alpha++;
    }
    //pthread_mutex_unlock(&memPtr->mutex);
}

void assignArr(Ptr &p,int idx,int val){
    if(p.type!=INT || p.type!=MEDIUM_INT)
        throw "Invalid type";
    //pthread_mutex_lock(&memPtr->mutex);
    cout<<"Assigning array elements"<<endl;
    int *alpha = (memPtr->memBase + symTable->stacks[p.idx].offset);
    alpha +=idx;
    *alpha = val;
    //pthread_mutex_unlock(&memPtr->mutex);
}

void assignArr(Ptr &p,int idx,char val){
    if(p.type!=CHAR)
        throw "Invalid type";
    //pthread_mutex_lock(&memPtr->mutex);
    cout<<"Assigning array elements"<<endl;
    char *alpha = (char *)(memPtr->memBase + symTable->stacks[p.idx].offset);
    alpha +=idx;
    *alpha = val;
    //pthread_mutex_unlock(&memPtr->mutex);
}

void assignArr(Ptr &p,int idx,bool val){
    if(p.type!=BOOLEAN)
        throw "Invalid type";
    //pthread_mutex_lock(&memPtr->mutex);
    cout<<"Assigning array elements"<<endl;
    bool *alpha =(bool *)(memPtr->memBase + symTable->stacks[p.idx].offset);
    alpha +=idx;
    *alpha = val;
    //pthread_mutex_unlock(&memPtr->mutex);
}


void freeElem(Ptr &p){
    cout<<"freeing memory with stack idx: "<<p.idx<<endl;
    int *alpha = memPtr->memBase + symTable->stacks[p.idx].offset;
    alpha--;
    *alpha = ((*alpha)>>1)<<1;    
}
void freeElem(int physical_offset){
    cout<<"freeing memory with physical_offset: "<<physical_offset<<endl;
    int *alpha = memPtr->memBase + physical_offset;
    alpha--;
    *alpha = ((*alpha)>>1)<<1;
}

void freeMem(){
    cout<<"Terminating the program"<<endl;
    pthread_kill(mythread,SIGINT);
    free(memPtr);
    free(symTable);
    free(mark);
}

void startScope(){
    symTable->stacks[symTable->idx++].offset = -1;
}

void stopScope(){
    for(int i = symTable->idx-1;i>=0;i--){
        if(symTable->stacks[i].offset == -1){
            symTable->idx = i;
            break;
        }
        pthread_mutex_lock(&mark->mutex);
        mark->arr[mark->idx++] = symTable->stacks[i].offset;
        pthread_mutex_unlock(&mark->mutex);
    }
}

void gc_initialize(){
    mark = new markArray();
}
void gc_run(){
    bool compress = false;
    pthread_mutex_lock(&mark->mutex);
    //cout<<"here"<<endl;
    if(mark->idx!=0){
        compress = true;
        //pthread_mutex_lock(&memPtr->mutex);
        for(int i=mark->idx-1;i>=0;i--)
            freeElem(mark->arr[i]);  
        mark->idx = 0;
        //pthread_mutex_unlock(&memPtr->mutex);
    }
    //cout<<"heter"<<endl;
    pthread_mutex_unlock(&mark->mutex);

    if(compress){
        //cout<<"heter"<<endl;
        pthread_mutex_lock(&memPtr->mutex);
        // TODO
        // merge multiple empty blocks together
        int temp = 0;
        int *alpha = (memPtr->memBase);
        bool update = false;
        int updateLoc = 0;
        while(temp<memPtr->size){
            if((*(alpha+temp))%2==0){
                if(update){
                    (*(alpha+updateLoc)) = ((((*(alpha+updateLoc))>>1)+((*(alpha+temp))>>1))<<1);
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
    //cout<<"here"<<endl;
}

void *garbageCollector(void *){
    gc_initialize();
    while(1){
        gc_run();
        usleep(1000);
    }
}  

void getArr(Ptr &p,int a[],int n){
    if(p.type!=INT && p.type!=MEDIUM_INT)
        throw "Invalid type";
    if(p.num_elements!=n)
        throw "Invalid size";
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
    bool *alpha = (bool *)(memPtr->memBase+symTable->stacks[p.idx].offset);
    for(int i=0;i<n;i++){
        a[i] = *alpha;
        alpha++;
    }
    
}

void getArr(Ptr &p,int idx,int &val){
    if(p.type!=INT && p.type!=MEDIUM_INT)
        throw "Invalid type";
    int *alpha = memPtr->memBase+symTable->stacks[p.idx].offset;
    alpha+=idx;
    val = *alpha;

}

void getArr(Ptr &p,int idx,char &val){
    if(p.type!=CHAR)
        throw "Invalid type";
    char *alpha = (char *)(memPtr->memBase+symTable->stacks[p.idx].offset);
    alpha+=idx;
    val = (*alpha);
}

void getArr(Ptr &p,int idx,bool &val){
    if(p.type!=BOOLEAN)
        throw "Invalid type";
    bool *alpha = (bool *)(memPtr->memBase+symTable->stacks[p.idx].offset);
    alpha+=idx;
    val = (*alpha);
}

/**
 * Ptr a = createVar(INT);
 * a.id -> symboltable[a.id] = memory location
 * 
 */