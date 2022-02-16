//Group 1
//OS Assignment 3
//19CS10031 - Gandhi Abhishek Rajesh
//19CS10051 - Sajal Chhamunya


#include <unistd.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <iostream>
#include <wait.h>
using namespace std;

typedef struct _process_data {
double **A;
double **B;
double **C;
int veclen, i, j;
} ProcessData;

void *mult(ProcessData *arg){
    arg->C[arg->i][arg->j] = 0;
    //cout<<"veclen = "<<arg->veclen<<endl;
    for(int j=0;j<arg->veclen;j++){
        arg->C[arg->i][arg->j] += arg->A[arg->i][j] * arg->B[j][arg->j];
        //cout<<arg->i<<" "<<arg->j<<" "<<arg->C[arg->i][arg->j]<<" "<<arg->A[arg->i][j]<<" "<<arg->B[j][arg->j]<<endl;
        //cout<<"val = "<<arg->C[arg->i][arg->j]<<endl;
    }
    return 0;
}

void create_index(void **m, int row, int col, size_t sizeElement){
    size_t sizeRow = col * sizeElement;
    m[0] = m+row;
    for(int i=1; i<row; i++){      
        m[i] = (void *)((size_t)m[i-1]+sizeRow);
    }
}

void print_matriz(double **matrix, int row, int col){
    printf("\n");
    for(int i=0; i<row; i++){
        for(int j=0; j<col; j++)
            printf("%.2f\t",matrix[i][j]);
        printf("\n");
    }
}


int main(){

    double **A, **B, **C;
    int r1,c1,c2,shmIdA,shmIdB,shmIdC;
    printf("Enter the rows and column of Matrix A and column of matrix B \n");
    cin>>r1>>c1>>c2;

    size_t sizeA = r1 * (sizeof(double *) + (c1 * sizeof(double)));
    size_t sizeB = c1 * (sizeof(double *) + (c2 * sizeof(double)));
    size_t sizeC = r1 * (sizeof(double *) + (c2 * sizeof(double)));

    //const int sz = sizeof(double) * (r1*c1 + c1*c2 + r1*c2);
    shmIdA = shmget(IPC_PRIVATE, (sizeA), IPC_CREAT|0666);   
    shmIdB = shmget(IPC_PRIVATE, (sizeB), IPC_CREAT|0666);   
    shmIdC = shmget(IPC_PRIVATE, (sizeC), IPC_CREAT|0666);    
    A = (double**)shmat(shmIdA, NULL, 0); 
    B = (double**)shmat(shmIdB, NULL, 0); 
    C = (double**)shmat(shmIdC, NULL, 0); 
    create_index((void**)A, r1, c1, sizeof(double));
    create_index((void**)B, c1, c2, sizeof(double));
    create_index((void**)C, r1, c2, sizeof(double));

    cout<<"Enter matrix A:\n";
    for(int i=0; i<r1; i++){
        for(int j=0; j<c1; j++){
            scanf("%lf",&A[i][j]);
        }
    }       
    cout<<"Enter matrix B:\n";
    for(int i=0; i<c1; i++){
        for(int j=0; j<c2; j++)
            cin>>B[i][j];
    }    
    


    ProcessData pd;
    pd.A = A;
    pd.B = B;
    pd.C = C;
    pd.veclen = c1;

    for (int i = 0; i < r1; ++i){
        for (int j = 0; j < c2; ++j){
            pd.i = i;
            pd.j = j;
            int pid = fork();
            if(pid<0){
                perror("error in pid");
            }
            else if (pid==0){
                mult(&pd);
                exit(0);   
            }            
            else{
                continue;
            }
        }
    }

    for (int i = 0; i < r1; ++i){
        for (int j = 0; j < c2; ++j){    
            wait(NULL);
        }
    }

    for(int i=0;i<r1;i++){
        for(int j=0;j<c2;j++){
            cout<<C[i][j]<<" ";
        }
        cout<<endl;
    }
    shmdt(A);
    shmdt(B);
    shmdt(C);
    shmctl(shmIdA,IPC_RMID, NULL);
    shmctl(shmIdB,IPC_RMID, NULL);
    shmctl(shmIdC,IPC_RMID, NULL);



    
    return 0;
}



