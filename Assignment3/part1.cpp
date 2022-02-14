
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/shm.h>
#include <fcntl.h>
#include <iostream>
using namespace std;


typedef struct _process_data {
double **A;
double **B;
double **C;
int veclen, i, j;
} ProcessData;

void *mult(ProcessData *arg){
    arg->C[arg->i][arg->j] = 0;
    for(int j=0;j<sizeof((arg->B));j++){
        arg->C[arg->i][arg->j] += arg->A[arg->i][j] * arg->B[j][arg->j];
    }
}

int main(){
    int r1,c1,r2; cin>>r1>>c1>>r2;
    
    return 1;
}