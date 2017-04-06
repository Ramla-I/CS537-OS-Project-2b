#ifndef __mem_h__
#define __mem_h__

#define E_NO_SPACE            (1)
#define E_CORRUPT_FREESPACE   (2)
#define E_PADDING_OVERWRITTEN (3)
#define E_BAD_ARGS            (4)
#define E_BAD_POINTER         (5)

#define BESTFIT				  (0)
#define WORSTFIT			  (1)
#define FIRSTFIT			  (2)

typedef struct {
	void* next;
	int size;
} list_t;

typedef struct {
	int magic;;
	int size;
} header;

extern int m_error;
extern list_t* freeList;

int Mem_Init(int sizeOfRegion);
void *Mem_Alloc(int size, int style);
int Mem_Free(void *ptr);
void Mem_Dump();


#endif // __mem_h__