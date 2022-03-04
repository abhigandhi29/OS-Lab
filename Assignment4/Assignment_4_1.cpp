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

int main(){
    return 0;
}