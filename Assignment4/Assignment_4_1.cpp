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
#include <semaphore.h>

using namespace std;


struct job {
    int id;
    int timeOfCompletion;
}; 
enum jobStatus {completed, onGoing, done};

struct Node { 
    job j; 
    jobStatus status;
    sem_t mutex;
    Node *parent;
    Node *child;
};

int producer(){
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

    for(int i = 0; i < P; i++){
        int pid = fork();
        if(pid<0){
            printf("Error in creating producer process. Exitting..\n");
			exit(1);
        }
        else if(pid==0){
            producer();
            exit(0);
        }
        else{
            continue;
        }
    }

    //for(int i = 0; i < C; i++){
    int pid = fork();
    if(pid<0){
        printf("Error in creating producer process. Exitting..\n");
        exit(1);
    }
    else if(pid==0){
        consumer(C);
        exit(0);
    }
        

    //}

    return 0;
}