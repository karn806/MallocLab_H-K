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
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7) // make it divisble by 8


#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

/* Basic constants and macros */
#define SWORD 4
/* Word and header/footer size (bytes) */
#define DWORD 8
/* Double word size (bytes) */
#define CHUNKSIZE (1<<12) /* Extend heap by this amount (bytes) */

#define MAX(x, y) ((x) > (y)? (x) : (y))

/* Pack a size and allocated bit into a word */
#define PACK(size, alloc) ((size) | (alloc))

/* Read and write a word at address p */
#define GET(p) (*(unsigned int *)(p))
#define PUT(p, val) (*(unsigned int *)(p) = (val))
#define PUT_PTR(p, ptr) (*(unsigned int **)(p) = (ptr))

/* Read the size and allocated fields from address p */
#define GET_SIZE(p) (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)

/* Given block ptr ptr, compute address of its header and footer */
#define HDRP(ptr) ((char *)(ptr) - SWORD)
#define FTRP(ptr) ((char *)(ptr) + GET_SIZE(HDRP(ptr)) - DWORD)

/* Given block ptr ptr, compute address of next and previous blocks */
#define NEXT_BLKP(ptr) ((char *)(ptr) + GET_SIZE(((char *)(ptr) - SWORD)))
#define PREV_BLKP(ptr) ((char *)(ptr) - GET_SIZE(((char *)(ptr) - DWORD)))

/* Given block ptr, compute adr of pred and suc word */
#define SUCC(ptr) ((char *)(ptr))
#define PRED(ptr) ((char *)(ptr+ SWORD))
#define debug_me() printf("%s:%d I'm here\n",__FILE__, __LINE__)
#define print_snp(bp) printf("suc : %x, pred: %x\n", GET(SUCC(bp)), GET(SUCC(bp)) );
#define print_hdrp(bp) printf("header adr: %x, size: %d, alloc: %d\n", HDRP(bp), GET_SIZE(HDRP(bp)), GET_ALLOC(HDRP(bp)) );

void *heap_listp;
void *root_listp;
static void *extend_heap(size_t size);
static void add_lst(void *ptr);
static void remove_lst(void *ptr);
/* 
 * mm_init - initialize the malloc package.
 */
 void mm_check(void){
    int bc = 0;
    void *pt;
    for (pt = heap_listp; (GET_SIZE(HDRP(pt)) != 0) ; pt = NEXT_BLKP(pt) ){
        printf("Header # %x, Size: %d, Alloc %d\n", pt, GET_SIZE(HDRP(pt)), GET_ALLOC(HDRP(pt)));
        printf("Footer # %x, Size: %d, Alloc %d\n", pt, GET_SIZE(FTRP(pt)), GET_ALLOC(FTRP(pt)));
        if (GET_ALLOC(HDRP(pt)) == 0){
            printf("BLK # %x, SUCC_PT: %x, PRED_PT: %x\n", pt, GET(SUCC(pt)), GET(PRED(pt)));
        }
        printf("Current root_listp is: %x, size: %d, alloc: %d\n", root_listp, GET_SIZE(HDRP(root_listp)), 
            GET_ALLOC(HDRP(root_listp)));
        bc++;
    }
    printf("---------------------\n");
}
int mm_init(void)
{
    debug_me();
    char *bp;

    /* Create the initial empty heap */
    if ((heap_listp = mem_sbrk(4*SWORD)) == (void *)-1)
        return -1;
    PUT(heap_listp, 0); /* Alignment padding */
    PUT(heap_listp + (1*SWORD), PACK(DWORD, 1)); /* Prologue header */
    PUT(heap_listp + (2*SWORD), PACK(DWORD, 1)); /* Prologue footer */
    PUT(heap_listp + (3*SWORD), PACK(0, 1)); /* Epilogue header */
    heap_listp += (2*SWORD);

    /* Extend the empty heap with a free block of CHUNKSIZE bytes */
    printf("Start init\n");
    if ( (bp = extend_heap(CHUNKSIZE/SWORD)) == NULL)
        return -1;
    PUT(SUCC(bp), NULL);
    PUT(PRED(bp), NULL);
    root_listp = bp;
    mm_check();
    printf("Done init()\n");
    return 0;


}

static void add_lst(void *ptr){
    void *temp_bp = root_listp;
    root_listp = ptr; //change root to new free block
    printf("Cur root: %x, size: %d, alloc: %d \n", root_listp, GET_SIZE(HDRP(root_listp)), 
        GET_ALLOC(HDRP(root_listp)));
    printf("current succ: %x, cur_pred: %x\n", SUCC(temp_bp), PRED(temp_bp));
    printf("current succ: %x, cur_pred: %x\n", SUCC(root_listp), PRED(root_listp) );
    if (GET(SUCC(ptr)) && GET(PRED(ptr))){
        debug_me();
        PUT(SUCC(ptr), temp_bp); //change current succ pointer to the recent free block
        debug_me();
        PUT(PRED(temp_bp), ptr); //change recent free block pred to the current new root ptr
    }
    printf("Add successful yeh!\n");
}

static void remove_lst(void *ptr){
    void *cur_pred = GET(PRED(ptr)); //get pointer of pred
    void *cur_suc = GET(SUCC(ptr)); // get pointer of suc
    if (cur_pred != NULL && cur_suc != NULL){
        PUT(cur_pred, cur_suc); // swap to merge them together
        PUT(cur_suc + SWORD, cur_pred);
    }
}

static void *coalesce(void *ptr)
{
    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(ptr)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(ptr)));
    size_t size = GET_SIZE(HDRP(ptr));

    if (prev_alloc && next_alloc) {                 /* Case 1 */
        printf("Go into case 1\n");
        add_lst(ptr);
        return ptr;
    }

    else if (prev_alloc && !next_alloc) {           /* Case 2 */
        size += GET_SIZE(HDRP(NEXT_BLKP(ptr)));
        printf("Go into case 2\n");
        remove_lst(NEXT_BLKP(ptr));
        PUT(HDRP(ptr), PACK(size, 0));
        PUT(FTRP(ptr), PACK(size,0));
        add_lst(ptr);
    }

    else if (!prev_alloc && next_alloc) {           /* Case 3 */
        printf("Go into case 3\n");
        size += GET_SIZE(HDRP(PREV_BLKP(ptr)));
        PUT(FTRP(ptr), PACK(size, 0));
        PUT(HDRP(PREV_BLKP(ptr)), PACK(size, 0));
        ptr = PREV_BLKP(ptr);
        remove_lst(ptr);
        add_lst(ptr);
    }

    else {                                          /* Case 4 */
        printf("Go into case 4\n");
        size += GET_SIZE(HDRP(PREV_BLKP(ptr))) +
        GET_SIZE(FTRP(NEXT_BLKP(ptr)));
        remove_lst(NEXT_BLKP(ptr));
        PUT(HDRP(PREV_BLKP(ptr)), PACK(size, 0));
        PUT(FTRP(NEXT_BLKP(ptr)), PACK(size, 0));
        ptr = PREV_BLKP(ptr);
        remove_lst(ptr);
        add_lst(ptr);
    }
    return ptr;
}

static void *extend_heap(size_t words)
{
    char *ptr;
    size_t size;

    /* Allocate an even number of words to maintain alignment */
    size = (words % 2) ? (words+1) * SWORD : words * SWORD;
    if ((long)(ptr = mem_sbrk(size)) == -1)
        return NULL;

    /* Initialize free block header/footer and the epilogue header */
    PUT(HDRP(ptr), PACK(size, 0));           /* Free block header */
    PUT(FTRP(ptr), PACK(size, 0));           /* Free block footer */
    PUT(HDRP(NEXT_BLKP(ptr)), PACK(0, 1));   /* New epilogue header */

    /* Coalesce if the previous block was free */
    printf("Before coalesce\n");
    return coalesce(ptr);
}

static void *find_fit(size_t asize)
{
    /* First fit search */
    void *ptr;
    for (ptr = heap_listp; GET_SIZE(HDRP(ptr)) > 0; ptr = NEXT_BLKP(ptr)) {
        if (!GET_ALLOC(HDRP(ptr)) && (asize <= GET_SIZE(HDRP(ptr)))) {
            return ptr;
        }
    }
    return NULL; /* No fit */
}

static void place(void *ptr, size_t asize)
{
    size_t csize = GET_SIZE(HDRP(ptr));

    if ((csize - asize) >= (2*DWORD)) {
        debug_me();
        PUT(HDRP(ptr), PACK(asize, 1));
        PUT(FTRP(ptr), PACK(asize, 1));
        void *old_pred = GET(PRED(ptr));
        void *old_succ = GET(SUCC(ptr));
        printf("old_pred: %x, old_succ: %x\n", old_pred, old_succ);
        /* pointer shift to the next alrdy */
        ptr = NEXT_BLKP(ptr);
        PUT(HDRP(ptr), PACK(csize-asize, 0));
        PUT(FTRP(ptr), PACK(csize-asize, 0));
        root_listp = ptr;
        if (old_pred != NULL && old_succ != NULL){
            debug_me();
            PUT(PRED(ptr), old_pred);
            PUT(SUCC(ptr), old_succ);
            print_hdrp(ptr);
            print_snp(ptr);
            print_hdrp(old_pred);
            PUT(old_pred, ptr);
            PUT(old_succ + SWORD, ptr);
            debug_me();
        }
        debug_me();
            
    }
    else {
        debug_me();
        PUT(HDRP(ptr), PACK(csize, 1));
        PUT(FTRP(ptr), PACK(csize, 1));
        remove_lst(ptr);
    }
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    debug_me();
    printf("malloc with size %d\n", size);
    size_t asize;       /* Adjusted block size */
    size_t extendsize;  /* Amount to extend heap if no fit */
    char *ptr;

    /* Ignore spurious requests */
    if (size == 0)
        return NULL;

    /* Adjust block size to include overhead and alignment reqs. */
    if (size <= DWORD)
        asize = 2*DWORD;
    else
        asize = DWORD * ((size + (DWORD) + (DWORD-1)) / DWORD);

    /* Search the free list for a fit */
    if ((ptr = find_fit(asize)) != NULL) {
        debug_me();
        place(ptr, asize);
        debug_me();
        return ptr;
    }
    /* No fit found. Get more memory and place the block */

    extendsize = MAX(asize,CHUNKSIZE);
    if ((ptr = extend_heap(extendsize/SWORD)) == NULL)
        return NULL;
    place(ptr, asize);
    return ptr;

}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
    size_t size = GET_SIZE(HDRP(ptr));
    PUT(HDRP(ptr), PACK(size, 0));
    PUT(FTRP(ptr), PACK(size, 0));
    coalesce(ptr);
}


/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{

    void *oldptr = ptr;
    void *newptr;
    size_t oldsize = GET_SIZE(HDRP(ptr));
    // size_t copySize;

    if (oldptr==NULL){
        return mm_malloc(size);
    }

    if (size==0){
        mm_free(ptr);
        // return 0
    }

    if (oldsize==size){
        return oldptr;
    }

    if (size < oldsize){
        newptr = mm_malloc(size);
        memcpy(newptr, oldptr, size);
        mm_free(oldptr);
    }

    if (size > oldsize){
        newptr = mm_malloc(size);
        memcpy(newptr, oldptr, oldsize);
        mm_free(oldptr);
    }

    return newptr;

}














