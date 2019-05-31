#include <unistd.h>
#include <stdio.h>
#include <malloc.h>
#include <pthread.h>



#define WORKE_SIZE  100
#define TIMES 100


int atomic_int_inc(int * addr,int delta){
//based on xaddl delta,[ptr]
	int ret =0;
	__asm__ __volatile__(
			"lock;xaddl %2,%1;"	
			:"=a"(ret)
			:"m"(*addr),"0"(delta) 
			:"cc","memory"
	);
	return ret;
}



//cas atomic opt  based on "cmpxchg new,[ptr]"
int atomic_int_cas(int* addr,int _old,int _new){
	int ret = 0;

	__asm__ __volatile__(
			"lock;cmpxchg %3,%1;"
			:"=a"(ret)
			:"m" (*addr),"0"(_old),"r"(_new)
			:"memory","cc"
	);
	
	return ret;
}


unsigned long atomic_ulong_cas(unsigned long* addr,unsigned long  _old,unsigned long _new){
	unsigned long ret = 0;

	__asm__ __volatile__(
			"lock;cmpxchg %3,%1;"
			:"=a"(ret)
			:"m" (*addr),"0"(_old),"r"(_new)
			:"memory","cc"
	);
	
	return ret;
}




struct Singleton_;
typedef struct Singleton_ Singleton;

struct Singleton_{
	void(*destruct)(Singleton*this);

};

static Singleton* ins=0;

Singleton* getInstance(){
	if (ins==NULL){
		  void *memptr = malloc(sizeof(Singleton));
		  if (0!=atomic_ulong_cas((long*)&ins,0,memptr)){//&ins,0,memptr 参数列表，0表示只会执行一次赋值
		     //因为old 固定 0，赋值操作只会发生一次，其余皆是无效操作，所以要释放 其它的memptr
		     //少部分 补偿措施
		    if(memptr!=0)
			{
				 free(memptr);
			     memptr = NULL;
		    }
		  }
	}           
		return ins;
}


void* workerRoutine(void* arg){
		void* in = getInstance();
		usleep(100);
		printf("ins= %p ...\n",ins);
	return 0;
}

void   __attribute__((destructor))   release(){
	if (ins!=0){
		free(ins);
		ins =0;
		}
}


int main(int argc,const char *argvs[]){

#if 0
       int *iptr,ivalue;
	   long *lptr,lvalue;
	   long long *llptr,llvalue;
	   
	   printf("iptr=%d ,ivalue=%d \n",sizeof(iptr),sizeof(ivalue));
	   printf("lptr=%d ,lvalue=%d \n",sizeof(lptr),sizeof(lvalue)); 
	   printf("llptr=%d ,llvalue=%d \n",sizeof(llptr),sizeof(llvalue));
		return 0;
		#endif

		pthread_t workers[WORKE_SIZE]={0};

		int i;
		for (i=0;i<WORKE_SIZE;++i){
			pthread_create(&workers[i],0,workerRoutine,0);
		}

		for (i=0;i<WORKE_SIZE;++i){
			pthread_join(workers[i],0);
		}



	return 0;
}















