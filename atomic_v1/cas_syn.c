#include <stdio.h>




int cas(int *addr,int old,int _new){
		if (*addr==old){
			(*addr)=_new;
			return _new;
		}
	return old;
}







//framework description  %1 %0 是 占位符 ， "0" 匹配符
//cmpxchg 涉及哪些寄存器 %1 和eax比较，


long cmpxchg_long(long *addr,long old,long new){
	int ret = 0;

	__asm__ __volatile__(
		"lock;cmpxchg %2,%1;"
		:"=a"(ret)
		:"m"(*addr),"r"(new),"0"(old)
		:"memory" ,"cc"
	);
	return ret;
}


//每个汇编指令，要知道它隐含使用的寄存器
//cmp and then  exchange
int cmpxchg_int(int  * ptr,int _old,int _new){
	int ret = 0;

	__asm__ __volatile__(
		"lock;cmpxchg %3,%1"    //隐含使用eax寄存器，%1和 eax比较 如果==，那么%3赋值给%1 （不管eax）,否则不等就，%1 覆盖eax
		:"=a"(ret)
		:"m"(*ptr),"0"(_old),"r" (_new)
		:"cc" ,"memory"
	);
	return ret;
}



int main(int argc,const char *argvs[]){
	int a = 1;
	int ret= cmpxchg_int(&a,a/**1或者2**/,a+1);//

//&a 和ptr 等价，这是不同的代码层面

//如果 当前内存值*(&a)== 当前线程栈中的值时候，那么就会发生(cmpxchg ecx,[ptr]), ecx的值去覆盖 [ptr]的值，ZF清0；
//如果 当前内存值*(&a)！=当前线程栈中的值(by copy into stack value),那么被别的并发线程修改的*(&a)的值 ，就拿去更新 eax

//当前测试使用了单线程，所以出现了，相等，保留了old的值，不等就是内存值（此时 内存值==old），看上面结果都一样
//实际上，在并发环境下，*(&a) ！=old 不等的情况下,都是被其它线程修改过的，那在并发环境下，这个*(&a) 就是被对方修改过的最新值

	
 	printf("ret=%d , a=%d \n",ret,a);
	
	return 0;
}





















