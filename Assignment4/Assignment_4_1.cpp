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


struct job {
    int id;
    int timeOfCompletion;
}; 
enum jobStatus {completed, onGoing, done};

struct Node { 
    int x;
    job j; 
    jobStatus status;
    //sem_t mutex;
    Node *parent;
    Node *child;
};

struct Tree{
    Node *n;
    int size;
};


void *producer(void* Runtime){
    int t = (int)Runtime;
    
    return 0;
}

int consumer(int y){
    return 0;
}

int main(){
    int P,C;
    cout<<"Enter number of Producer: ";
    cin>>P;
    cout<<"Enter number of Consumer: ";
    cin>>C;

    int shmid = shmget(IPC_PRIVATE,sizeof(Node)*100*P,0666|IPC_CREAT);
    if(shmid<0){
		printf("Error in creating SHM! try again..\n");
		exit(1);
	}

    Node *n= (Node *)shmat(shmid,NULL,0);

    //for(int i = 0; i < C; i++){
    pthread_t mythreads[P]; // identifier for the thread that we will create
    pthread_attr_t attr; // will store attributes of the thread (e.g., stack size)
    pthread_attr_init (&attr); // get default attributes

    for(int i=0;i<P;i++){
        int t = (rand() % 10000 + 10000);
        pthread_create(&mythreads[i], &attr, producer, (void *)t);
    }

    int pid = fork();
    if(pid<0){
        printf("Error in creating producer process. Exitting..\n");
        exit(1);
    }
    else if(pid==0){
        consumer(C);
        exit(0);
    }
    //else{
    //    producer();
    //}
        

    //}

    return 0;
}