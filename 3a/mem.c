#include "mem.h"
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

list_t* freeList = NULL;
int m_error = -1;

int 
Mem_Init(int sizeOfRegion)
{
	printf("In mem_init\n");

	//error checking
	if(sizeOfRegion <= 0)
	{
		m_error= E_BAD_ARGS;
		return -1; 
	}
	else if(freeList != NULL)
	{
		m_error= E_BAD_ARGS;
		return -1; 
	}

	//find page size and make sure requested is a multiple
	int sz = getpagesize();
	int numOfPages = 0;

	if(sizeOfRegion%sz != 0)
	{
		numOfPages = (sizeOfRegion/sz) +1;
		sizeOfRegion = numOfPages*sz;
	}	
	
	//allocate memory
	int fd = open("/dev/zero", O_RDWR);
	void* ptr = mmap(NULL, sizeOfRegion, PROT_READ|PROT_WRITE, MAP_PRIVATE, fd, 0);
	if(ptr == MAP_FAILED) 
	{
		perror("mmap"); 
		return -1;
	}

	close(fd);

	//initialize free list
	freeList = (list_t*) ptr;
	freeList->size = sizeOfRegion- sizeof(list_t);
	freeList->next = NULL;
	
	//printf("%llx\n", (long long unsigned) freeList);
	//printf("%d\n",sizeOfRegion);
	//printf("%d\n",sizeof(list_t));
	//printf("%d\n",freeList->size );
	//printf("header size: %d\n List size: %d\n", sizeof(header), sizeof(list_t));
	return 0;

}

void*
Mem_Alloc(int size, int style)
{
	printf("In mem_alloc\n");

	if(size <= 0)
	{
		m_error = E_BAD_ARGS;
		return NULL;
	}

	int actual_size = size + sizeof(header);
	//check if requested size is multiple of 8
	int x = 8;
	if(actual_size%x != 0)
	{
		actual_size = ((actual_size/x) +1) *8;
		size  = actual_size - sizeof(header);
	}

	list_t* region;
	list_t* ptr;
	list_t* previous;
	list_t* previous_reg;
	char* traversal;
	header* newRegion;
	void* final;

	region = freeList;
	ptr = freeList;
	while(ptr->size < actual_size)
	{
		ptr = (list_t*)ptr->next;
	}
	if(ptr == NULL)
	{
		m_error = E_NO_SPACE;
		return NULL;
	}

	previous_reg = NULL;
	previous = NULL;

	//find region according to style
	if(style == BESTFIT)
	{
		while(region != NULL)
		{
			if(region->size >= actual_size)
			{
				if(region->size < ptr->size)
				{
					previous = previous_reg;
					ptr = region;
				}	

			}
			previous_reg = region;
			region = region->next;
		}
	}

	else if(style == WORSTFIT)
	{
		while(region != NULL)
		{
			if(region->size >= actual_size)
			{
				if(region->size > ptr->size)
				{
					previous = previous_reg;
					ptr = region;
				}	

			}
			previous_reg = region;
			region = region->next;
		}
	}

	else if (style == FIRSTFIT)
	{
		int a = 0;
		while (a != 1)
		{
			if(region->size >= actual_size)
			{
				previous = previous_reg;
				ptr = region;	
				a = 1;
			}
			previous_reg = region;
			region = region->next;
		}

	}

	else
	{
		m_error = E_BAD_ARGS;
		return NULL;
	}
	//End of finding region

	//check if allocated region is at head of the list because initialized with head of list
	if(ptr == freeList)
	{
		/*if(freeList->size < actual_size)
		{
			m_error = E_NO_SPACE;
			return NULL;
		}
		else*/
		if(freeList->size == actual_size)
		{
			freeList = freeList->next;
		}

		else
		{			
			//find ending point of allocated region
			traversal = (char*) ptr;
			traversal = traversal + actual_size;
			freeList = (list_t*) traversal;
			freeList->next = ptr->next;
			freeList->size = ptr->size - actual_size;
		}

		//allocate region and make header
			newRegion = (header*) ptr;
			newRegion->size = size;
			newRegion-> magic = 1111;
			final = newRegion+1;	

	}
	//allocated region is not at head of list
	else
	{
		//find ending point of allocated region
		traversal = (char*) ptr;
		traversal = traversal + actual_size;
		list_t* a = (list_t*) traversal;
		a->next = ptr->next;
		a->size = ptr->size - actual_size;
		previous->next = (void*)a;

		//allocate region and make header
		newRegion = (header*) ptr;
		newRegion->size = size;
		newRegion-> magic = 1111;
		final = newRegion+1;			

	}


	return final;

}


int
Mem_Free(void *ptr)
{
	printf("In mem_free\n");

	if(ptr == NULL)
	{
		m_error = E_BAD_POINTER;
		return -1;
	}

	header* free_region  = (void*)ptr - sizeof(header);
	int size = free_region->size + sizeof(header);

	//beginning of free regiom
	void* region = (void*) free_region;
	//region after region to be freed
	list_t* traverse = freeList;
	//region before region to be freed
	list_t* previous = NULL;

	//find region right before and right after region to be freed
	while((region > (void*) traverse)&&(traverse!=NULL))
	{
		previous = traverse;
		traverse = (list_t*) traverse->next;
	}

	//check if free list is empty
	if((traverse == NULL)&&(previous == NULL))
	{
		freeList = (list_t*)region;
		freeList->size = size - sizeof(list_t);
		freeList->next = NULL;
		return 0;
	}

	//first region of free list is after free region
	else if (previous == NULL)
	{
		void* end_free = region + size;
		//check coalescing
		if((end_free) == (void*)traverse)
		{
			list_t* new = (list_t*) region;
			new->size = size + traverse->size;
			new->next = traverse->next;
			freeList = new;
		}
		else
		{
			list_t* new = (list_t*) region;
			new->size = size -sizeof(list_t);
			new->next = (void*)traverse;
			freeList = new;
		}

	}

	//region of free list are all before free space
	else if (traverse == NULL)
	{
		void* end_prev = (void*)previous + sizeof(list_t) + previous-> size;
		//check coalescing
		if((end_prev) == region)
		{
			previous->size = previous->size + size;
		}
		else
		{
			list_t* new = (list_t*) region;
			new->size = size -sizeof(list_t);
			new->next = NULL;
			previous->next = (void*) new;
		}

	}

	//somewhere between two regions
	else
	{
		void* end_prev = (void*)previous + sizeof(list_t) + previous-> size;
		void* end_free = region + size;

		//check if can combine with previous region
		if((end_prev) == region)
		{
			previous->size = previous->size + size;

			//check if can combine with next region as well
			if((end_free) == (void*)traverse)
			{
				previous->size = previous->size + sizeof(list_t) + traverse->size;
				previous->next = traverse->next;
			}
		}
		//check if can combine with next region
		else if((end_free) == (void*)traverse)
		{
			list_t* new = (list_t*) region;
			new->size = size + traverse->size;
			new->next = traverse->next;
		}
		//can't be combined
		else
		{
			previous->next = region;
			list_t* new = (list_t*)region;
			new-> next = (void*) traverse;
			new->size = size - sizeof(list_t);
		}


	}


	return 0;

}

void
Mem_Dump()
{
	printf("In mem_dump\n");

	list_t* ptr;
	ptr = freeList;
	while(ptr!=NULL)
	{
		printf("%llx\n", (long long unsigned) ptr);
		printf("%d\n", ptr->size);
		printf("%llx\n\n", (long long unsigned) ptr->next);
		ptr = ptr->next;
	}

}