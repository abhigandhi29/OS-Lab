//Group 1
//OS Assignment 4
//19CS10031 - Gandhi Abhishek Rajesh
//19CS10051 - Sajal Chhamunya


#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/shm.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
//#include <semaphore.h>
#include <chrono>
#include <pthread.h>

using namespace std;
using namespace std::chrono;

#define MAX_CHILD 100


struct job {
    int id;
    int timeOfCompletion;
}; 
enum jobStatus {notScheduled=0, Scheduled=1, done=2};

struct Node { 
    int x;
    job j; 
    jobStatus status;
    //sem_t mutex;
    pthread_mutex_t mutex;
    int parent;
    int child[MAX_CHILD];
    int depend_cnt;
};

struct Tree{
    pthread_mutex_t mutex;
    //pthread_mutex_t print_mutex;
    Node *node;
    int size;
    int currIdx;
    int status;
};

struct ProducerData {
    int runTime;
    Tree *tree;
    ProducerData(int runTime, Tree *tree) : runTime(runTime),tree(tree){}
};

int findRandom(Tree *tree, int idx,double prob){
    Node *n = tree->node; 
    double t  = (double(rand()%100)/100.0);
    if(n[idx].status!=notScheduled) return -1;
    if(n[idx].depend_cnt<MAX_CHILD && t<prob){
        return idx; 
    } 
    for(int i=0; i<MAX_CHILD; i++){
        if(n[idx].child[i]>0){
            int t = findRandom(tree,n[idx].child[i],prob);
            if(t>=0) return t;
        }
    }
    return -1;
}

int getFirstChild(Tree* T, int ind){
    Node *N = T->node;
    if(N[ind].status!=0){
        cout<<ind<<" "<<N[ind].status<<endl;
        return -1;
    }
    else if(N[ind].depend_cnt==0){
        cout<<"here "<<ind<<endl;
        if(T->status==0 && ind==0)
            return -1;
        return ind;
    }
    for(int i=0;i<MAX_CHILD;i++){
        if(N[ind].child[i]>=0){
            int j=getFirstChild(T,N[ind].child[i]);
            if(j!=-1)
                return j;
        }
        
    }

    //cout<<"this one "<<ind<<" "<<N[ind].status<<" "<<N[ind].depend_cnt<<endl;
    return -1;
}
int makeChild(Tree *Tr,int idx,int parent){
    Node* n = Tr->node;
    n[idx].parent = parent;
    n[idx].j.id = rand()%((int)(1e8))+1;
    n[idx].j.timeOfCompletion = rand()%(250)+1;
    n[idx].status = notScheduled;
    n[idx].depend_cnt = 0;
    //n[idx].active = 1;
    for(int i=0;i<MAX_CHILD;i++){
        n[idx].child[i] = -1;
    }
    if(parent != -1){
        for(int i=0;i<MAX_CHILD;i++){
            if(n[parent].child[i] == -1){
                n[parent].child[i] = idx;
                n[parent].depend_cnt++;
                break;
            } 
        }
    }
    int sem_job = pthread_mutex_init(&(n[idx].mutex),NULL);
    if (sem_job < 0){
        perror("Could not create semaphore\n");
        exit(1);
    }
    //pthread_mutex_lock(&Tr->print_mutex);
    cout<<"New Job Created, Job id: "<<n[idx].j.id<<" "<<" with index: "<<idx<<" and with parent idx: "<<parent<<endl;
    //pthread_mutex_unlock(&Tr->print_mutex);
    return 0;
}

void *producer(void* p){
    ProducerData *producer = (ProducerData *)p;
    int t = producer->runTime;
    //time_point<system_clock> start, end;
  
    auto start = high_resolution_clock::now();
    double elapsed_time_ms = duration<double, std::milli>(high_resolution_clock::now()-start).count();
    while(elapsed_time_ms < t){
        usleep(rand()%(300000)+200000);
        cout<<"T = "<<t<<" "<<elapsed_time_ms<<endl;
        
        if(producer->tree->size==0){
            pthread_mutex_lock(&producer->tree->mutex);
            if(producer->tree->size==0){
                int idx = producer->tree->currIdx++;
                makeChild(producer->tree,idx,-1);
                producer->tree->size++;
                //cout<<"here"<<endl;
            }
            pthread_mutex_unlock(&producer->tree->mutex);
            elapsed_time_ms = duration<double, std::milli>(high_resolution_clock::now()-start).count();
            continue;
        }
        
        int idx = -1;
        while(idx==-1 && elapsed_time_ms<t){
            idx = findRandom(producer->tree,0,0.1);
            // int id = rand()%(producer->tree->currIdx);
            // if(producer->tree->node[id].status == notScheduled && producer->tree->node[id].depend_cnt<MAX_CHILD && producer->tree->node[id].j.id>0){
            //     idx = id;
            //     break;
            // }
            elapsed_time_ms = duration<double, std::milli>(high_resolution_clock::now()-start).count();
            cout<<"--------------------------------"<<endl;
        }
        if(elapsed_time_ms>=t) break;
        cout<<"here1"<<" "<<idx<<endl;
        pthread_mutex_lock(&producer->tree->node[idx].mutex);
        cout<<"here2"<<endl;
        if(producer->tree->node[idx].status != notScheduled || producer->tree->node[idx].depend_cnt>=MAX_CHILD){
            cout<<"here3"<<endl;
            pthread_mutex_unlock(&producer->tree->node[idx].mutex);
            elapsed_time_ms = duration<double, std::milli>(high_resolution_clock::now()-start).count();
            continue;
        }
        cout<<"here4"<<endl;
        pthread_mutex_lock(&producer->tree->mutex);
        int idn = producer->tree->currIdx++;
        pthread_mutex_unlock(&producer->tree->mutex);
        makeChild(producer->tree,idn,idx);
        producer->tree->size++;
        pthread_mutex_unlock(&producer->tree->node[idx].mutex);
        
        cout<<"here5"<<endl;
        elapsed_time_ms = duration<double, std::milli>(high_resolution_clock::now()-start).count();
        
    }
    cout<<"terminated"<<endl;
    pthread_exit(0); 
    //return 0;
}


void* consumerProcess(void* T){
    Tree * Tr=(Tree *)T;
    Node* arr=Tr->node;
    while(1){
        cout<<"Size = "<<Tr->size<<" "<<Tr->status<<" "<<Tr->node[0].status<<endl;
        if(Tr->status==1 && Tr->size==0)
            pthread_exit(0);     
            //return 0;
        if(Tr->size==0)
            continue;
        if(Tr->size==1 && Tr->status==0)
            continue;
        int ind=getFirstChild(Tr,0);
        //cout<<ind<<endl;
        
        if(ind==-1){
            continue;
            //return 0;
        }
        cout<<"consumer "<<ind<<endl;
        pthread_mutex_lock(&Tr->node[ind].mutex);
        if(Tr->node[ind].depend_cnt!=0 || Tr->node[ind].status!=notScheduled){
            pthread_mutex_unlock(&Tr->node[ind].mutex);
            continue;
        }
        //pthread_mutex_lock(&Tr->print_mutex);
        cout<<"Start Execution of New Job, Job id: "<<arr[ind].j.id<<" with index: "<<ind<<endl;
        //pthread_mutex_unlock(&Tr->print_mutex);

        arr[ind].status=Scheduled;
        usleep(arr[ind].j.timeOfCompletion*1000);
        arr[ind].status=done;
        int parent = arr[ind].parent;
        if(parent!=-1){
            for(int i=0; i<MAX_CHILD;i++){
                if(arr[parent].child[i]==ind){
                    arr[parent].child[i] = -1;
                    arr[parent].depend_cnt--;
                    break;
                }
            }
        }
        (Tr->size)--;
        
        
        //pthread_mutex_lock(&Tr->print_mutex);
        cout<<"Completion of Job, Job id: "<<arr[ind].j.id<<endl;
        //pthread_mutex_unlock(&Tr->print_mutex);
        pthread_mutex_unlock(&Tr->node[ind].mutex);
        cout<<"end consumer"<<endl;
    }
    return 0;
}

int consumer(Tree* T,int y){
    pthread_t mythreads[y]; // identifier for the thread that we will create
    pthread_attr_t attr; // will store attributes of the thread (e.g., stack size)
    pthread_attr_init (&attr); // get default attributes



    for(int i=0;i<y;i++){
        pthread_create(&mythreads[i], &attr, consumerProcess, (void *)T);
    }
    for(int i=0;i<y;i++){
        pthread_join(mythreads[i],NULL);
    }
    cout<<"consumer ending"<<endl;
    return 0;
}





int main(){
    int P,C;
    cout<<"Enter number of Producer: ";
    cin>>P;
    cout<<"Enter number of Consumer: ";
    cin>>C;

    srand(time(0));
    int shmIdNode = shmget(IPC_PRIVATE,sizeof(Node)*(500+100*P),0666|IPC_CREAT);
    if(shmIdNode<0){
		printf("Error in creating SHM! try again..\n");
		exit(1);
	}

    Node *n= (Node *)shmat(shmIdNode,NULL,0);

    int shmIdTree = shmget(IPC_PRIVATE,sizeof(Tree),0666|IPC_CREAT);
    if(shmIdTree<0){
		printf("Error in creating SHM! try again..\n");
		exit(1);
	}

    Tree *tree= (Tree *)shmat(shmIdTree,NULL,0);
    tree->node = n;
    tree->size=0;
    tree->currIdx = 0;
    tree->status = 0;
    int sem_job = pthread_mutex_init(&(tree->mutex),NULL);
    if (sem_job < 0){
        perror("Could not create semaphore\n");
        exit(1);
    }
    // sem_job = pthread_mutex_init(&(tree->print_mutex),NULL);
    // if (sem_job < 0){
        // perror("Could not create semaphore\n");
        // exit(1);
    // }

    makeChild(tree,tree->currIdx++,-1);
    tree->size++;
    int max_size=rand()%200+300;
    for(int i=0;i<max_size;i++){
        int idx = -1;
        while(idx==-1){
            int id = rand()%(tree->size);
            if(n[id].depend_cnt<MAX_CHILD){
                idx = id;
                break;
            }
        }
        
        makeChild(tree,tree->currIdx++,idx);
        tree->size++;
    }
    

    //for(int i = 0; i < C; i++){
    pthread_t mythreads[P]; // identifier for the thread that we will create
    pthread_attr_t attr; // will store attributes of the thread (e.g., stack size)
    pthread_attr_init (&attr); // get default attributes

    for(int i=0;i<P;i++){
        int t = (rand() % 10000 + 10000);
        ProducerData *p = new ProducerData(t,tree);
        pthread_create(&mythreads[i], &attr, producer, (void *)p);
    }
    

    int pid = fork();
    if(pid<0){
        printf("Error in creating producer process. Exitting..\n");
        exit(1);
    }
    else if(pid==0){
        consumer(tree,C);
        exit(0);
    }

    // for(int i=0;i<P;i++){
    //     pthread_join(mythreads[i],NULL);
    // }
    sleep(20);
    tree->status = 1;
    //wait(NULL);
    //else{
    //    producer();
    //}
        

    //}

    return 0;
}