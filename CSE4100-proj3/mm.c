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
 * provide your information in the following struct.
 ********************************************************/
team_t team = {
    /* Your student ID */
    "20181605",
    /* Your full name*/
    "Kim Sanghyeon",
    /* Your email address */
    "anton3017@sogang.ac.kr",
};

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT - 1)) & ~0x7)
#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

// Basic constants and macros
#define WORDSIZE 4
#define DWORDSIZE 8
#define CHUNKSIZE (1 << DWORDSIZE)
#define LISTLIMIT 13

#define MAX(x, y) ((x) > (y) ? (x) : (y))
// pack a size and allocated bit into a word
#define PACK(size, alloc) ((size) | (alloc))

// Read and wirte a word at address p
#define GET(p) (*(unsigned int *)(p))
#define PUT(p, val) (*(unsigned int *)(p) = (val))

// Read the size and allocated fields from address p
#define GET_SIZE(p) (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)

// Given block ptr bp, compute address of its header and footer
#define HTAGP(bp) ((char *)(bp)-WORDSIZE)
#define FTAGP(bp) ((char *)(bp) + GET_SIZE(HTAGP(bp)) - DWORDSIZE)

// Given block ptr bp, compute address of next and previous blocks
#define NEXT_BLOCKP(bp) ((char *)(bp) + GET_SIZE(((char *)(bp)-WORDSIZE)))
#define PREV_BLOCKP(bp) ((char *)(bp)-GET_SIZE(((char *)(bp)-DWORDSIZE)))

// Given block find previous or next free block in explicit list
#define PRED_FREE(bp) (*(void **)(bp))
#define SUCC_FREE(bp) (*(void **)((char *)bp + WORDSIZE))

void *seg_list[LISTLIMIT];

void *extend_heap(size_t words);
void *coalesce(void *bp);
void *first_fit(size_t asize);
void place(void *bp, size_t asize);
void remove_block(void *bp);
void insert_block(void *bp, size_t size);

/*
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    void *bp;

    for (int list = 0; list < LISTLIMIT; list++)
    {
        seg_list[list] = NULL;
    }

    if ((bp = mem_sbrk(4 * WORDSIZE)) == (void *)-1)
    {
        return -1;
    }
    PUT(bp, 0);
    PUT(bp + (1 * WORDSIZE), PACK(DWORDSIZE, 1));
    PUT(bp + (2 * WORDSIZE), PACK(DWORDSIZE, 1));
    PUT(bp + (3 * WORDSIZE), PACK(0, 1));

    return 0;
}

/*
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    int asize = ALIGN(size + SIZE_T_SIZE);
    size_t padded_size;
    char *bp;

    if (!(bp = first_fit(asize)))
    {
        padded_size = MAX(asize, CHUNKSIZE);
        if (!(bp = extend_heap(padded_size / WORDSIZE)))
        {
            return NULL;
        }
    }
    place(bp, asize);
    return bp;
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
    size_t size = GET_SIZE(HTAGP(ptr));

    PUT(HTAGP(ptr), PACK(size, 0));
    PUT(FTAGP(ptr), PACK(size, 0));

    coalesce(ptr);
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
    if (!newptr)
        return NULL;
    copySize = GET_SIZE(HTAGP(oldptr));
    if (size < copySize)
        copySize = size;

    memcpy(newptr, oldptr, copySize);
    mm_free(oldptr);
    return newptr;
}

void *extend_heap(size_t words)
{
    char *bp;
    size_t size;

    size = (words % 2) ? (words + 1) * WORDSIZE : words * WORDSIZE;
    if ((int)(bp = mem_sbrk(size)) == -1)
    {
        return NULL;
    }
    PUT(HTAGP(bp), PACK(size, 0));
    PUT(FTAGP(bp), PACK(size, 0));
    PUT(HTAGP(NEXT_BLOCKP(bp)), PACK(0, 1));

    return coalesce(bp);
}

void *first_fit(size_t asize)
{
    void *bp;
    int list_idx;

    for (list_idx = 1; list_idx < LISTLIMIT; list_idx++)
    {
        if (asize >= (1 << (list_idx - 1)))
        {
            bp = seg_list[list_idx];
            while (bp && (asize > GET_SIZE(HTAGP(bp))))
            {
                bp = SUCC_FREE(bp);
            }
            if (bp)
            {
                return bp;
            }
        }
    }

    return NULL;
}

void *coalesce(void *bp)
{
    size_t prev_alloc = GET_ALLOC(FTAGP(PREV_BLOCKP(bp)));
    size_t next_alloc = GET_ALLOC(HTAGP(NEXT_BLOCKP(bp)));
    size_t size = GET_SIZE(HTAGP(bp));

    if (prev_alloc)
    {
        if (next_alloc)
        {
            insert_block(bp, size);
            return bp;
        }
        else
        {
            remove_block(NEXT_BLOCKP(bp));

            size += GET_SIZE(HTAGP(NEXT_BLOCKP(bp)));
            PUT(HTAGP(bp), PACK(size, 0));
            PUT(FTAGP(bp), PACK(size, 0));
        }
    }
    else
    {
        if (next_alloc)
        {
            remove_block(PREV_BLOCKP(bp));

            size += GET_SIZE(HTAGP(PREV_BLOCKP(bp)));
            PUT(FTAGP(bp), PACK(size, 0));
            PUT(HTAGP(PREV_BLOCKP(bp)), PACK(size, 0));
            bp = PREV_BLOCKP(bp);
        }
        else
        {
            remove_block(PREV_BLOCKP(bp));
            remove_block(NEXT_BLOCKP(bp));

            size += GET_SIZE(HTAGP(PREV_BLOCKP(bp))) + GET_SIZE(FTAGP(NEXT_BLOCKP(bp)));
            PUT(HTAGP(PREV_BLOCKP(bp)), PACK(size, 0));
            PUT(FTAGP(NEXT_BLOCKP(bp)), PACK(size, 0));
            bp = PREV_BLOCKP(bp);
        }
    }

    insert_block(bp, size);
    return bp;
}

void place(void *bp, size_t asize)
{
    size_t csize = GET_SIZE(HTAGP(bp));

    remove_block(bp);

    if ((csize - asize) >= (2 * DWORDSIZE))
    {
        PUT(HTAGP(bp), PACK(asize, 1));
        PUT(FTAGP(bp), PACK(asize, 1));
        bp = NEXT_BLOCKP(bp);

        PUT(HTAGP(bp), PACK(csize - asize, 0));
        PUT(FTAGP(bp), PACK(csize - asize, 0));
        coalesce(bp);
    }
    else
    {
        PUT(HTAGP(bp), PACK(csize, 1));
        PUT(FTAGP(bp), PACK(csize, 1));
    }
}

void insert_block(void *bp, size_t size)
{
    int list = 0;
    void *search_ptr;
    void *insert_ptr = NULL;

    while ((list < LISTLIMIT - 1) && (size > 1))
    {
        size >>= 1;
        list++;
    }

    search_ptr = seg_list[list];
    while (search_ptr && (size > GET_SIZE(HTAGP(search_ptr))))
    {
        insert_ptr = search_ptr;
        search_ptr = SUCC_FREE(search_ptr);
    }

    if (search_ptr)
    {
        if (insert_ptr)
        {
            SUCC_FREE(bp) = search_ptr;
            PRED_FREE(bp) = insert_ptr;
            PRED_FREE(search_ptr) = bp;
            SUCC_FREE(insert_ptr) = bp;
        }
        else
        {
            SUCC_FREE(bp) = search_ptr;
            PRED_FREE(bp) = NULL;
            PRED_FREE(search_ptr) = bp;
            seg_list[list] = bp;
        }
    }
    else
    {
        if (insert_ptr)
        {
            SUCC_FREE(bp) = NULL;
            PRED_FREE(bp) = insert_ptr;
            SUCC_FREE(insert_ptr) = bp;
        }
        else
        {
            SUCC_FREE(bp) = NULL;
            PRED_FREE(bp) = NULL;
            seg_list[list] = bp;
        }
    }
    return;
}

void remove_block(void *bp)
{
    int list = 0;
    size_t size = GET_SIZE(HTAGP(bp));

    while ((list < LISTLIMIT - 1) && (size > 1))
    {
        size >>= 1;
        list++;
    }

    if (SUCC_FREE(bp))
    {
        if (PRED_FREE(bp))
        {
            PRED_FREE(SUCC_FREE(bp)) = PRED_FREE(bp);
            SUCC_FREE(PRED_FREE(bp)) = SUCC_FREE(bp);
        }
        else
        {
            PRED_FREE(SUCC_FREE(bp)) = NULL;
            seg_list[list] = SUCC_FREE(bp);
        }
    }
    else
    {
        if (PRED_FREE(bp))
        {
            SUCC_FREE(PRED_FREE(bp)) = NULL;
        }
        else
        {
            seg_list[list] = NULL;
        }
    }
    return;
}