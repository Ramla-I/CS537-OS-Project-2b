#include<stdio.h>
#include "mem.h"

int main()
{
	void* ptr1, *ptr2, *ptr3;
	Mem_Init(1000);

	ptr1 = Mem_Alloc(16,0);
	//Mem_Dump();
	//printf("%llx\n", (long long unsigned) ptr1);
	ptr2 = Mem_Alloc(20,0);
	//Mem_Dump();
	//printf("%llx\n", (long long unsigned) ptr2);
	//Mem_Free(ptr1);
	//Mem_Dump();
	ptr3 = Mem_Alloc(16,2);
	//printf("%llx\n", (long long unsigned) ptr3);

	Mem_Free(ptr1);
	Mem_Dump();

	Mem_Free(ptr2);
	Mem_Dump();

	Mem_Free(ptr3);
	Mem_Dump();
	return 0;
}
