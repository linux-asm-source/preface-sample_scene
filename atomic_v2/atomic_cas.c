#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

#include "./base/fwbase.h"


//打开打印

#define OPEN_PRINT 0

//打开原子操作

#define ATOMIC 1

int atomic_int_inc(int* addr,int delta){
	//based on asm : xaddl delta,[ptr] 
	int ret = 0;
// input output sys arch
	__asm__ __volatile__(
		"lock; xaddl %2,%1;"
		:"=a"(ret)
		:"m"(*addr) ,"a"(delta)
		:"memory","cc"

	);
	return ret;

}


//used to  condition assign
int atomic_int_cas(int * addr,int _old,int _new){
	int ret =0;
//asm  :output  :input : destroy segment
	//use asm   cmpxchg new,[ptr]   ; first [ptr] cmp with eax (oldvalue) ,if cmp result is equ then 
	__asm__ __volatile__(
	"lock;cmpxchg %3,%1;"
	:"=a"(ret)
	:"m"(*addr),"0"(_old),"r"(_new)
	:"memory","cc"
);
	return ret;
}



long atomic_long_cas(long* addr,long _old,long _new){
	long ret =0;
//asm  :output  :input : destroy segment
	//use asm   cmpxchg new,[ptr]   ; first [ptr] cmp with eax (oldvalue) ,if cmp result is equ then 
	__asm__ __volatile__(
	"lock;cmpxchg %3,%1;"
	:"=a"(ret)
	:"m"(*addr),"0"(_old),"r"(_new)
	:"memory","cc"
);
	return ret;
}





#define WORKERS_SIZE 100
#define TIMES 100


static void* completionProc(void *arg){
	int * value = (int*)arg;
	int i;
	for (i=0;i<TIMES;++i){
		#if ATOMIC
		
		atomic_int_inc(value,1);
		
		#else
		
		(*value)++;
		
	    #endif
 		
		usleep(100);
	}

	(void*)0;
}

struct Singleton_;
typedef struct Singleton_ Singleton;

struct Singleton_{
	void (*destruct)(Singleton*);
};

static Singleton * ins=0;


void SingletonDestruct(Singleton*);
Singleton* SingletonConstruct(Singleton* this){

	this->destruct = SingletonDestruct;

	return this;
}

void SingletonDestruct(Singleton* this){

	printf("release singleton....\n");
}

static void* printV(void *arg){
	while(1){

		usleep(1000);
		printf("uuid=%d \n",*((int*)arg));
	};


	(void*)0;
}
static void* printIns(void *arg){
	while(1){

		usleep(1000);
		printf("ins=%p \n",*((Singleton**)arg));
	};


	(void*)0;
}


Singleton * getInstance(){
	//目前是old value 一直是0，也就是old(0 是一个固定值)的永远得不到刷新，也就意味着，
	//也就意味着，这个赋值操作，只会执行一次

	Singleton* memPtr =0;
	if (ins==0){
			    memPtr = NEW(Singleton);
		       //第一次执行 默认ins==0 ，和old 一样，会发生一次赋值，返回是0；再次执行，ins非0,old 还是0 ，所以返回的是 memptr的指针值，也就是重复 赋值过程，即后面马上free操作		
				//note tip: cas执行和条件判断if 合一  
				if (NULL!=(Singleton*) atomic_long_cas((long*)&ins, 0, (long*)memPtr)){//补偿性删除，因为只有返回null，才说明是第一次，否则就是第2次以上调用了生成了新的内存;
					VIRTUAL_DELETE(memPtr);
					 
					};
				}
	return ins;	
}




void *completionGetInstance(void * arg){
	  void* ins = getInstance();
	  usleep(300);
	  printf("instance=%p \n",ins);

	return (void*)0;
}


void __attribute__ ((destructor)) release(){
	VIRTUAL_DELETE(ins);

}




//整合cas 单例  baseframework

int main(int argc,const char *argvs[]){

	pthread_t workers[WORKERS_SIZE]={0};

 	int i;
	int uuid= 0;
	for (i=0;i<WORKERS_SIZE;++i){
				pthread_create(&workers[i],0,completionGetInstance,&uuid);
	}

	for (i=0;i<WORKERS_SIZE;++i){
					pthread_join(workers[i],0);
	}



#if OPEN_PRINT
	pthread_t inspect_worker;
	pthread_create(&inspect_worker,0,printIns,&ins);

	pthread_join(inspect_worker,0);

#endif
	return 0;

}
















