#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

// xaddl delta,eax
//redis will use this
static int atomic_int_inc(int * addr,int addValue){
	int ret=0;

	__asm__ __volatile__(
		"lock;xaddl %2,%1;"
		:"=a"(ret)
		:"m"(*addr),"0"(addValue)  //or :"m"(*addr),"a"(addValue)
		:"memory" ,"cc"
	);

	return ret;
}

//读的维度可以非常多
//写的维度可以非常少

/****

struct Queue{

	int head;
	int tail;

	int size;
	struct task *tks;

};

****/


#define TIMES 100
#define WORKER_SIZE 10


void* pthreadProc(void* arg){
    int i,res=0;
	int *value = (int *)arg;
	for (i=0;i<TIMES;++i){
		//(*value)++;
		res=atomic_int_inc(value,1);//user atomic to syn 
		printf("res=%d...\n",res);
		usleep(500);
	}
	return (void*)0;
}



void * printRoutine(void* arg){
    int * printV =(int*) arg;
	while(1){
		printf("total=%d\n",*printV);
	}
	return (void*)0;
}


int main(int argc,const char *argvs[]){
	pthread_t workers[WORKER_SIZE]={0};
	int total =0;
	int i;
	int printFlag=1;
	for (i=0;i<WORKER_SIZE;++i){
		pthread_create(&workers[i],NULL,pthreadProc,&total);
	}

/****pthread_t printWorker;
	pthread_create(&printWorker,NULL,printRoutine,&total);
	pthread_join(printWorker,0);
****/

	for (i=0;i<WORKER_SIZE;++i){
		pthread_join(workers[i],0);
	}
	
	
	

	return 0;
}

















