//Group 1
//OS Assignment 3
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

#define MAX_QUEUE_SIZE 8
#define MAX_JOB_ID 100000
#define N 10
#define int long long

typedef struct job{
	int matrix[N][N];
	int proNo;
	int matId;
	int status;
	//0: D000 will be computed, 1: D001 , 2: D010, 3:D011 ...and so on
	//8: computation complete 
    job(int proNo):proNo(proNo),matId(rand()%MAX_JOB_ID+1),status(0)
    {
        for (int i = 0; i < N; ++i){
		    for (int j = 0; j < N; ++j){
			    matrix[i][j] = rand()%19 - 9;
		    }
	    }
    }
    job(int proNo, int mat[N][N]):proNo(proNo),matId(rand()%MAX_JOB_ID+1),status(0)
    {
        for (int i = 0; i < N; ++i){
		    for (int j = 0; j < N; ++j){
			    matrix[i][j] = mat[i][j];
		    }
	    }
    }
}job;

typedef struct shared_memory{
	job job_queue[MAX_QUEUE_SIZE];
	int size;
	int job_created;
	int tot_matrix;
	//int tot_jobs; //no to worker process required = (tot_matrix-1)*8
	int computed;
	int status;
	sem_t mutex;
	sem_t full;
	sem_t empty;
}shared_memory;


int start_A_x[] = {0,0,N/2,N/2,0,0,N/2,N/2};
int start_A_y[] = {0,N/2,0,N/2,0,N/2,0,N/2};
int start_B_x[] = {0,N/2,0,N/2,0,N/2,0,N/2};
int start_B_y[] = {0,0,0,0,N/2,N/2,N/2,N/2};
int start_C_x[] = {0,0,N/2,N/2,0,0,N/2,N/2};
int start_C_y[] = {0,0,0,0,N/2,N/2,N/2,N/2};


void mult(shared_memory* shm, int ans[N][N], int status){
    for(int i=0;i<N;i++){
        for(int j=0;j<N;j++){
            ans[i][j]=0;
        }
    }
	// cout<<"status = "<<status<<endl;
    for(int i=0;i<N/2;i++){
        for(int j=0;j<N/2;j++){
            for(int k=0;k<N/2;k++){
                ans[i+start_C_x[status]][j+start_C_y[status]] += (shm->job_queue[0].matrix)[i+start_A_x[status]][k+start_A_y[status]]*(shm->job_queue[1].matrix)[k+start_B_x[status]][j+start_B_y[status]]; 
            }
        }
    }
}

bool insert_job(shared_memory* shm,job x){
	if(shm->size==MAX_QUEUE_SIZE){
		printf("Overflow: Cannoot insert\n");
		return false;
	}
	shm->job_queue[shm->size] = x;
	shm->size++;
    return true;
}

bool remove_job(shared_memory* shm){
	if(shm->size<=1){
		printf("Not enough job to remove\n");
		return false;
	}
	
	for (int i = 0; i < shm->size-2; ++i){
		shm->job_queue[i] = shm->job_queue[i+2];
	}
    shm->size-=2;
    return true;
}

bool add_end_job(shared_memory* shm, int C[N][N]){
    if(shm->size==0)
        return false;
    for(int i=0;i<N;i++){
        for(int j=0;j<N;j++){
            (shm->job_queue[shm->size-1]).matrix[i][j]+=C[i][j];
        }
    }
    return true;
}

void print_job(job x){
	printf("Job ID: %d\n",getpid());
	printf("producer: %lld\n",x.proNo);
	for (int i = 0; i < N; ++i){
		for (int j = 0; j < N; ++j){
			printf("%lld  ",x.matrix[i][j]);
		}
		printf("\n");
	}
}


void producer(int shmid,int proNo){
	shared_memory* shmp=(shared_memory*)shmat(shmid,NULL,0);
	while(1){
		// if all jobs created, exit
		if(shmp->job_created==shmp->tot_matrix)
			break;
		// create job
		job j(proNo);
		// random delay
        // cout<<"time="<<3*double(rand())/RAND_MAX<<endl;
		sleep(3*double(rand())/RAND_MAX);

		// wait for empty semaphore
		
		// if all jobs created, exit
		sem_wait(&(shmp->mutex));
		if(shmp->job_created>=shmp->tot_matrix){
			//sem_post(&(shmp->mutex));
			break;
		}
		
		
		if(shmp->size < MAX_QUEUE_SIZE-1){
            
		    // wait for mutex
		    sem_wait(&(shmp->empty));
			insert_job(shmp,j);
			
            printf("Produced job details:\n");
			print_job(j);
			// increment shared variable
			shmp->job_created++;
			// signal the full semaphore
            sem_post(&(shmp->full));	
            	
								
		}
		sem_post(&(shmp->mutex));	
		// signal mutex
		
	}
	// detach this process from shared memory
	shmdt(shmp);
}
// worker function
void worker(int shmid,int cons_no){
	shared_memory* shmc=(shared_memory*)shmat(shmid,NULL,0);
	while(1){
		// random delay

		sleep(3*double(rand())/RAND_MAX);
		// wait to acquire mutex
		sem_wait(&(shmc->mutex));
		// flag to indicate a job is retrieved
		int job_retrieved=0,status;
		if(shmc->size>1){
			//compute cij acc to status
            if(shmc->job_queue[0].status<8){
                status = shmc->job_queue[0].status;
                shmc->job_queue[0].status++;
                shmc->job_queue[1].status++;
                job_retrieved=1;
            }
		}
        sem_post(&(shmc->mutex));
		// signal mutex
		
		if(job_retrieved){            
            int ans[N][N];
			cout<<"Worker Number: "<<cons_no<<endl;
			cout<<"Producer Numbers: "<<shmc->job_queue[0].proNo<<endl;
			cout<<"Matrix ID: "<<shmc->job_queue[0].matId<<endl;
			cout<<"Work Done (with Block Numbers): ";
			if (status==0){
				cout<<"Reading (A00 and B00) and Copying (C00)"<<endl;
			}
			else if(status==1){
				cout<<"Reading (A01 and B10) and Adding (C00)"<<endl;
			}
			else if(status==2){
				cout<<"Reading (A10 and B00) and Copying (C10)"<<endl;
			}
			else if(status==3){
				cout<<"Reading (A11 and B10) and Adding (C10)"<<endl;
			}
			else if(status==4){
				cout<<"Reading (A00 and B01) and Copying (C01)"<<endl;
			}
			else if(status==5){
				cout<<"Reading (A01 and B11) and Adding (C01)"<<endl;
			}
			else if(status==6){
				cout<<"Reading (A10 and B01) and Copying (C11)"<<endl;
			}
			else{
				cout<<"Reading (A11 and B11) and Adding (C11)"<<endl;
			}


            mult(shmc,ans,status);
            
            //shmc->job_queue[0].status++;
			//shmc->job_queue[1].status++;
            sem_wait(&(shmc->mutex));
			if(shmc->status==0){
                job J(cons_no,ans);
                sem_wait(&(shmc->empty));
				
                insert_job(shmc,J);
                sem_post(&(shmc->full));
                //sem_post(&(shmc->mutex));
				shmc->status++;
            }
            else{
                //sem_wait(&(shmc->mutex));
				//sem_wait(&(shmc->mutex));
                add_end_job(shmc,ans);
                shmc->status++;
                if(shmc->status==8){
					//shmc->status = 0;
                    shmc->computed++;
                }
                
            }
			sem_post(&(shmc->mutex));
            

			// signal mutex
			//sem_post(&(shmc->mutex));
			// signal empty semaphore
			//sem_post(&(shmc->empty));
			// to ensure worker is killed only after it has slept/computed job
			
            //sem_wait(&(shmc->mutex));

			
			//sem_post(&(shmc->mutex));
			// cout<<"here"<<endl;
		}
		// cout<<"sizeeee="<<shmc->size<<endl;
        // cout<<"end"<<endl;
        sem_post(&(shmc->mutex));
        //if(shmc->computed==){
        //}
	}
	// detach from shared memory
	shmdt(shmc);
}

int32_t main(){
    srand(time(0));
	//create SHM
	key_t key = ftok("/dev/random",'c');
	if (key<0){
		printf("Error in generating key! try again..\n");
		exit(1);
	}
	int shmid = shmget(IPC_PRIVATE,sizeof(shared_memory),0666|IPC_CREAT);
	if(shmid<0){
		printf("Error in creating SHM! try again..\n");
		exit(1);
	}

	int NP,NW,tot_matrix;	
	cout<<"No workers:\n"; 
    cin>>NW;
	cout<<"No of Producers:\n"; 
    cin>>NP;
	cout<<"No of matrices to multiply:\n"; 
    cin>>tot_matrix;
	//initialize values in SHM
	shared_memory* shm = (shared_memory*)shmat(shmid,NULL,0);
	shm->size = 0;
	shm->tot_matrix = tot_matrix;
	shm->job_created = 0;
	int tot_jobs= (tot_matrix-1)*8;
	shm->computed = 0;
	shm->status = 0;
	
	
	// initialize the semaphore mutex
	//binary semaphore for access to jobs_created, jobs_completed, insertion & retrieval of jobs
	int sema = sem_init(&(shm->mutex),1,1);
	//counting semaphore to check if the job_queue is full
	int full_sema = sem_init(&(shm->full),1,0);
	//counting semaphore to check if the job_queue is empty
	int empty_sema= sem_init(&(shm->empty),1,MAX_QUEUE_SIZE);
	if(sema<0||full_sema<0||empty_sema<0){
		printf("Error in initializing semaphore. Exitting..\n");
		exit(1);
	}

	time_t start = time(0);
	pid_t pid;
	//producer
	pid_t pps[NP];
	for(int i=1;i<=NP;i++){
		pid=fork();
		if(pid<0){
			printf("Error in creating producer process. Exitting..\n");
			exit(1);
		}
		else if(pid==0){
			srand(time(0)+i);
			producer(shmid,i);
			return 0;
		}
		else{
			pps[i-1]=pid;
		}
	}
	//worker
	pid_t wps[NW];
	for(int i=1;i<=NW;i++){
		pid=fork();
		if(pid<0){
			printf("Error in creating worker process. Exitting..\n");
			exit(1);
		}
		else if(pid==0){
			srand(time(0)+NP+i);
			worker(shmid,NP+i);
			return 0;
		}
		else{
			wps[i-1]=pid;
		}
	}

	// loop till all jobs are created and consumed
	while(1){
		// acquire lock so that while checking, state change not possible
        sem_wait(&(shm->mutex));
		if(shm->job_queue[0].status==8 && shm->job_queue[1].status==8 && shm->status==8){
			sem_wait(&(shm->full));
			sem_wait(&(shm->full));
			remove_job(shm);
			sem_post(&(shm->empty));
			sem_post(&(shm->empty));
			shm->status = 0;
		}
		// shm->computed ensures that worker gets killed only after it has computed/slept
		if(shm->job_created>=tot_matrix && shm->size==1){
			time_t end = time(0);
			int time_taken = end-start;
            //cout<<"here"<<endl;
			printf("\n\n\033[1;32mProcess Completed\033[0m\nTime taken to run %lld jobs = %lld seconds\n",tot_matrix,time_taken);
			int diagonal_sum=0;
			for(int i=0;i<N;i++){
				diagonal_sum+=shm->job_queue[0].matrix[i][i];
			}
			cout<<"Sum of the elements in the principal diagonal of the final matrix: "<<diagonal_sum<<endl;
			// kill all child processes
			for(int i=0;i<NP;i++)
				kill(pps[i],SIGINT);
			for(int i=0;i<NW;i++)
				kill(wps[i],SIGINT);
			sem_post(&(shm->mutex));
			break;		
		}
		sem_post(&(shm->mutex));
	}
	//destroy mutex semaphore
	sem_destroy(&(shm->mutex));
	shmdt(shm);//detach shared memory segment
	shmctl(shmid,IPC_RMID,0);//mark shared memory segment to be destroyed
	return 0;
}