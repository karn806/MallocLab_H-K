/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 * 
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "Ham n Karn",
    /* First member's full name */
    "Worapol Boontanonda",
    /* First member's email address */
    "agentzh2m@gmail.com",
    /* Second member's full name (leave blank if none) */
    "Nattakarn Klongyut",
    /* Second member's email address (leave blank if none) */
    "nattakarn.kl@gmail.com"
};

/* single word (4) or double word (8) alignment */
#define SWORD 4
#define DWORD 8

/* Pack a size of allocated bit into a word */
#define PACK(size, alloc) ((size) | (alloc))

/* read and write a word at addr p */
#define GET(p) (*(unsigned int *)(p))
#define PUT(p, val) (*(unsigned int *)(p) = (val))

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (DWORD-1)) & ~0x7)
#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

/* read the size of allocated fields from addr p */
#define GET_SIZE(p) (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)

/* Given block ptr bp, compute addr of its header and footer */
#define HDRP(bp) ((char *)(bp) - SWORD)
#define FTRP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp)) - DWORD)

/* Given block ptr bp, compute addr of NEXT and PREV (only on free block ALLOC = 0) */
#define NEXT(bp) ((char * )(bp))
#define PREV(bp) ((char * )(bp) + SWORD)

/* Given block ptr bp, compute addr of next and previous block */
#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE(((char *)(bp) - DWORD)))
#define PRE_BLKP(bp) ((char *)(bp) - GET_SIZE(((char *)(bp) - DWORD)))

#define CHUNKSIZE (1 << 12)

void *heap_p;
void *root_p;
static void *extend_heap(size_t words) {
    char *bp;
    size_t size;
    if (words % 2 == 0){
        size = words;
    }else{
        size = words + 1;
    }
    if((bp = mem_sbrk(size)) == -1)
        return NULL;
    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
    PUT(NEXT(bp), NULL);
    PUT(PREV(bp), NULL);
    root_p = bp;
    PUT(NEXT_BLKP(bp), PACK(0, 1));
    return (void *) bp;
    coalesce(bp);

}

/* Check the block individually best use to see if init properly */
int mm_precheck(void){
    int bc = 0;
    void *pt;
    for (pt = heap_p; pt < heap_p + (2 * DWORD); pt+= SWORD){
        printf("Current block num is %d Alloc : %d, Size : %d \n",bc, GET(pt) & 0x1, GET(pt) & ~0x7);
        //printf("Current block footer num is %d Alloc : %d, Size : %d \n", bc, GET_ALLOC(FTRP(pt)), GET_SIZE(FTRP(pt)));
        bc++;
    }

}
/* Check Header and footer of all Free or unallocated block */
int mm_check(void){
	int bc = 0;
	void *pt;
	for (pt = heap_p + DWORD; GET_SIZE(FTRP(pt)) != 0 && GET_ALLOC(FTRP(pt)) != -1 ; pt = NEXT_BLKP(pt)){
		printf("Header # %d, Size: %d, Alloc %d\n", bc, GET_SIZE(HDRP(pt)), GET_ALLOC(HDRP(pt)));
		printf("Footer # %d, Size: %d, Alloc %d\n", bc, GET_SIZE(FTRP(pt)), GET_ALLOC(FTRP(pt)));
		/* print prev and next pointer if GETALLOC = 0 */
		if (GET_ALLOC(HDRP(pt)) == 0){
			printf("NEXT # %d, PT: %x \n", bc, GET(NEXT(pt)));
			printf("PREV # %d, PT: %x\n", bc, GET(PREV(pt)));
		}
	}
}

static void *coalesce(void *bp){
	/* freeing with LIFO policy */
	size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
	size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
	size_t size = GET_SIZE(HDRP(bp));
	/*Case 1 Insert the free block at the root of the list (when next and prev block is not free) */
	if (prev_alloc && next_alloc){
		PUT(NEXT(bp), root_p); //asign next to root of the lst
		PUT(PREV(bp), NULL); //set pred as null
		PUT(PREV(root_p), bp); //point the prev of root_p to current bp
		root_p = bp; //make the newly freed block as the root of the free lst
		return bp;
	}

	/*Case 2 when Next block is free but prev is not free, cut out successor block and insert new 
	block at the root of the lst */
	else if (!next_alloc && prev_alloc){
		size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
		PUT(HDRP(bp), PACK(size , 0));
		PUT(FTRP(bp), PACK(size , 0));
		/* join the prev block because it is gone now */
		void * next_bpt = NEXT_BLKP(bp);
		PUT(NEXT(NEXT_BLKP(bp)), bp);
		PUT(NEXT(bp), root_p);
		PUT(PREV(bp), NULL);
		PUT(PREV(root_p) , bp);
		root_p = bp;
		return bp;
	}

	/*Case 3 When Prev block is free but next block is not free, cut predecessor block and insert new block 
	at the root of the lst */

	else if (!prev_alloc && next_alloc){
		size+= GET_SIZE(HDRP(PREV_BLKP(bp)));
		PUT(FTRP(bp), PACK(size, 0));
		PUT(FTRP(bp), PACK(size, 0));
		bp = PREV_BLKP(bp)

	}

	/*Case 4 When Prev and Next bp is free Splice predecessor and successor blocks, coalesce all 3 mem
	block together and insert the new block at the root of the lst */
	else {
		size += GET_SIZE(HDRP(PREV_BLKP(bp))) + GET_SIZE(FTRP(NEXT_BLKP(bp)));
	}
}

/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    if ((heap_p = mem_sbrk(4 * SWORD)) == -1)
        return -1; 
    PUT(heap_p + (1 * SWORD), PACK(DWORD, 1));
    PUT(heap_p + (2 * SWORD), PACK(DWORD, 1));
    PUT(heap_p + (3 * SWORD), PACK(0,1));
    printf("Before Extend\n");
    mm_precheck();
    heap_p += 2 * SWORD;
    if ((extend_heap(CHUNKSIZE/SWORD)) == NULL) {
        return -1;
    }
    printf("After Extend\n");
    mm_check();
    return 0;
}


/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
	size_t asize;
	size_t extendsize;
	char *bp;
	/*Ignore stupid req */
	if (size == 0)
		return NULL;
	/* adjust block size to include overhead and alignment req */
	if (asize <= DWORD)
		asize = 2*DWORD;
	else
		asize = DWORD * ((size + DWORD + (DWORD - 1)) / DWORD); //amount of Double Word Block times the size of the block

	/*implement explict alloc*/

    int newsize = ALIGN(size + SIZE_T_SIZE);
    void *p = mem_sbrk(newsize);
    if (p == (void *)-1)
	return NULL;
    else {
        *(size_t *)p = size;
        return (void *)((char *)p + SIZE_T_SIZE);
    }
}

/*
* mm_free - Freeing a block does nothing.
*/
 
void mm_free(void *ptr)
{
	// size_t size = GET_SIZE(HDRP(bp));
	// PUT(HDRP(bp), PACK(size , 0));
	// PUT(FTRP(bp), PACK(size, 0));
	
	//coalesce(bp)
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
    void *oldptr = ptr;
    void *newptr;
    size_t copySize;
    
    newptr = mm_malloc(size);
    if (newptr == NULL)
      return NULL;
    copySize = *(size_t *)((char *)oldptr - SIZE_T_SIZE);
    if (size < copySize)
      copySize = size;
    memcpy(newptr, oldptr, copySize);
    mm_free(oldptr);
    return newptr;
}














