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

/* Gicen block ptr bp, compute addr of next and previous block */
#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE(((char *)(bp) - DWORD)))
#define PRE_BLKP(bp) ((char *)(bp) - GET_SIZE(((char *)(bp) - DWORD)))

/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    void *stptr = mem_sbrk(16);
    stptr = (int *) (stptr + 1);
    PUT_HEAD(stptr, 2, 1);
    stptr = (int *) (stptr + 1);
    PUT_HEAD(stptr, 2, 1);
    PUT_FOOT(stptr, 0, 1);
    return 0;
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    mm_check();
    int newsize = ALIGN(size + SIZE_T_SIZE);
    void *p = mem_sbrk(newsize);
    if (p == (void *)-1)
	return NULL;
    else {
        *(size_t *)p = size;
        return (void *)((char *)p + SIZE_T_SIZE);
    }
}

int mm_check(void){
	int *strpt = (int *)mem_heap_lo();
	int *lastpt = (int *)mem_heap_hi();
	int bc = 0;
	int *pt;
	for (pt = strpt + 1; pt <= lastpt; pt++) {
		printf("Current block pt is %d bpt is on %x",bc,  &pt);
		//printf("Header block info: Allocated: %d, Alloc_Size: %d", ALLOCATED(HEAD(pt)), ALLOC_SIZE(HEAD(pt)));
		//printf("Footer block info: Allocated: %d, Alloc_Size: %d", ALLOCATED(FOOT(pt)), ALLOC_SIZE(FOOT(pt)));
		bc++;
	}

}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
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














