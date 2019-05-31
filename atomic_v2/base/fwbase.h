#ifndef __FW_BASE__H
#define __FW_BASE__H

#include <string.h>
#include <malloc.h>

static void* myalloc(int size){
	void* ret =0;
	if (size==0) return 0;
	do{
		ret = malloc(size);
	}while(ret==0);
	memset(ret,0,size);

	return ret;
}

#if __WIN32 || __WIN64

#define NEW(type,...)\
type##Construct((type*)myalloc(sizeof(type)),##__VA_ARGS__);

#elif __linux__ || __unix__
#define NEW(type,...)\
({\
	type* ret = (type*)myalloc(sizeof(type));\
	type##Construct(ret,##__VA_ARGS__);\
	ret;\
})



#define VIRTUAL_DELETE(ptr)\
		do{\
			if (ptr!=0){\
				if (ptr->destruct!=NULL){\
					ptr->destruct(ptr);\
				}\
				free(ptr);\
				ptr=0;\
			}\
		}while(0);

#endif




#endif

















