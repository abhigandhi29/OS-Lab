
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/shm.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
using namespace std;

#define MAX_SIZE 2

typedef struct _process_data {
double A[MAX_SIZE][MAX_SIZE];
double B[MAX_SIZE][MAX_SIZE];
double C[MAX_SIZE][MAX_SIZE];
int veclen, i, j;
} ProcessData;

typedef struct _process_job {
int veclen, i, j;
} Job;

void *mult(ProcessData *arg,Job *J){
    arg->C[J->i][J->j] = 0;
    //cout<<"veclen = "<<arg->veclen<<endl;
    for(int j=0;j<J->veclen;j++){
        arg->C[J->i][J->j] += arg->A[J->i][j] * arg->B[j][J->j];
        //cout<<"val = "<<arg->C[arg->i][arg->j]<<endl;
    }
    return 0;
}


int main(){
    int r1,c1,c2; cin>>r1>>c1>>c2;
    int j;
    ProcessData *pData;
    int sh_id;
    // pData->A = allot_matrix(r1,c1,&sh_id_a);
    // pData->B = allot_matrix(c1,c2,&sh_id_b);
    // pData->C = allot_matrix(r1,c2,&sh_id_c);
    key_t keyfile = rand();

    if((sh_id = shmget(keyfile,sizeof(ProcessData),IPC_CREAT|0666)) == -1) {
        perror("shmget");
        exit(1);
    }

    if ((void*)(pData = (ProcessData*)shmat(sh_id, NULL, 0)) == (void *) -1) {
        perror("shmat");
        cout<<"erre"<<endl;
        exit(1);
    }

    
    for (int i = 0;i<r1;i++){
        for(int j = 0;j<c1;j++){
            cin>>pData->A[i][j];
        }
    }
    for (int i = 0;i<c1;i++){
        for(int j = 0;j<c2;j++){
            cin>>pData->B[i][j];
        }
    }
    Job *J = new Job();
    J->veclen = c1;
    for(int i=0; i<r1;i++){
        for(int j=0; j<c2;j++){
            pid_t pid = fork();
            if(pid == 0){
                //cout<<"her"<<endl;
                if ((void*)(pData = (ProcessData*)shmat(sh_id, NULL, 0)) == (void *) -1) {
                    perror("shmat");
                    //cout<<"erre"<<endl;
                    exit(1);
                }
                J->i = i;
                J->j = j;
                mult(pData,J);
                exit(1);
            }
        }
    }
    sleep(1);
    for(int i = 0;i<r1;i++){
        for(int j = 0;j<c2;j++){
            cout<<pData->C[i][j]<<" ";
        }
        cout<<endl;
    }

    return 1;
}