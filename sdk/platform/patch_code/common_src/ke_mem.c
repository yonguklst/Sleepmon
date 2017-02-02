/**
 ****************************************************************************************
 *
 * @file ke_mem.c
 *
 * @brief Implementation of the heap management module.
 *
 * Copyright (C) RivieraWaves 2009-2013
 *
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup MEM
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include <stdint.h>             // standard definition
#include <stdbool.h>            // standard boolean
#include "arch.h"               // architecture

#include "co_math.h"            // computation utilities
#include "ke_config.h"          // kernel configuration
#include "ke_env.h"             // kernel environment
#include "ke_mem.h"             // kernel memory

uint8_t ke_mem_heaps_used_local = KE_MEM_BLOCK_MAX;

/*
 * MACRO DEFINITIONS
 ****************************************************************************************
 */

/// number used to check if memory block has been corrupted or not
#define KE_NOT_LIST_CORRUPTED  0xA55A
#define KE_NOT_BUFF_CORRUPTED  0x8338
#define KE_NOT_BUFF_FREE_CORRUPTED  0xF00F

/*
 * LOCAL TYPE DEFINITIONS
 ****************************************************************************************
 */
/// Free memory block delimiter structure (size must be word multiple)
/// Heap can be represented as a doubled linked list.
struct mblock_free
{
    /// Next free block pointer
    struct mblock_free* next;
    /// Previous free block pointer
    struct mblock_free* previous;
    /// Size of the current free block (including delimiter)
    uint16_t free_size;
    /// Used to check if memory block has been corrupted or not
    uint16_t corrupt_check;
};

/// Used memory block delimiter structure (size must be word multiple)
struct mblock_used
{
    /// Size of the current used block (including delimiter)
    uint16_t size;
    /// Used to check if memory block has been corrupted or not
    uint16_t corrupt_check;
};

#if (LOG_MEM_USAGE)
extern struct mem_usage_log mem_log[KE_MEM_BLOCK_MAX];
#endif

/*
 * LOCAL FUNCTION DEFINITIONS
 ****************************************************************************************
 */

/**
 * Check if memory pointer is within heap address range
 *
 * @param[in] type Memory type.
 * @param[in] mem_ptr Memory pointer
 * @return True if it's in memory heap, False else.
 */
static bool ke_mem_is_in_heap(uint8_t type, void* mem_ptr)
{
    bool ret = false;
    uint8_t* block = (uint8_t*)ke_env.heap[type];
    uint32_t size = ke_env.heap_size[type];

    if((((uint32_t)mem_ptr) >= ((uint32_t)block))
            && (((uint32_t)mem_ptr) <= (((uint32_t)block) + size)))
    {
        ret = true;
    }

    return ret;
}

void *log_ke_malloc(uint32_t size, uint8_t type)
{
    struct mblock_free *node = NULL,*found = NULL;
    uint8_t cursor = 0;
    struct mblock_used *alloc = NULL;
    uint32_t totalsize;
    uint16_t heap_id = 0;

    // compute overall block size (including requested size PLUS descriptor size)
    totalsize = CO_ALIGN4_HI(size) + sizeof(struct mblock_used);
    if(totalsize < sizeof(struct mblock_free))
    {
        totalsize = sizeof(struct mblock_free);
    }

    // sanity check: the totalsize should be large enough to hold free block descriptor
    ASSERT_ERR(totalsize >= sizeof(struct mblock_free));

    // protect accesses to descriptors
    GLOBAL_INT_DISABLE();

    while((cursor < ke_mem_heaps_used_local)&& (found == NULL))
    {
        heap_id = ((cursor + type) % KE_MEM_BLOCK_MAX);
        #if KE_PROFILING
        uint32_t totalfreesize = 0;
        #endif //KE_PROFILING

        // Select Heap to use, first try to use current heap.
        node = ke_env.heap[heap_id];
        ASSERT_ERR(node != NULL);

        // go through free memory blocks list
        while (node != NULL)
        {
            ASSERT_ERR(node->corrupt_check == KE_NOT_LIST_CORRUPTED);
            #if KE_PROFILING
            totalfreesize += node->free_size;
            #endif //KE_PROFILING

            // check if there is enough room in this free block
            if (node->free_size >= (totalsize))
            {
                if ((node->free_size >= (totalsize + sizeof(struct mblock_free)))
                        || (node->previous != NULL))
                {
                    // if a match was already found, check if this one is smaller
                    if ((found == NULL) || (found->free_size > node->free_size))
                    {
                        found = node;
                    }
                }
            }

            // move to next block
            node = node->next;
        }

        // Update size to use complete list if possible.
        if(found != NULL)
        {
            if (found->free_size < (totalsize + sizeof(struct mblock_free)))
            {
                totalsize = found->free_size;
            }
        }

        #if KE_PROFILING
        ke_env.heap_used[heap_id] = ke_env.heap_size[heap_id] - totalfreesize;
        if(found != NULL)
        {
            ke_env.heap_used[heap_id] += totalsize;
        }
        #endif //KE_PROFILING

        // increment cursor
        cursor ++;
    }

    #if KE_PROFILING
    {
        // calculate max used size
        uint32_t totalusedsize = 0;
        for(cursor = 0 ; cursor < KE_MEM_BLOCK_MAX ; cursor ++)
        {
            totalusedsize +=  ke_env.heap_used[cursor];
        }

        if(ke_env.max_heap_used < totalusedsize)
        {
            ke_env.max_heap_used = totalusedsize;
        }
    }
    #endif //KE_PROFILING

    // Re-boot platform if no more empty space
    if(found == NULL)
    {
//        ASSERT_ERR(found != NULL);
        platform_reset(RESET_MEM_ALLOC_FAIL);
    }

    else
    {
    // sublist completly reused
    if (found->free_size == totalsize)
    {
        ASSERT_ERR(found->previous != NULL);

        // update double linked list
        found->previous->next = found->next;
        if(found->next != NULL)
        {
            found->next->previous = found->previous;
        }

        // compute the pointer to the beginning of the free space
        #if CPU_WORD_SIZE == 4
        alloc = (struct mblock_used*) ((uint32_t)found);
        #elif CPU_WORD_SIZE == 2
        alloc = (struct mblock_used*) ((uint16_t)found);
        #endif
    }
    else
    {
        // found a free block that matches, subtract the allocation size from the
        // free block size. If equal, the free block will be kept with 0 size... but
        // moving it out of the linked list is too much work.
        found->free_size -= totalsize;

        // compute the pointer to the beginning of the free space
        #if CPU_WORD_SIZE == 4
        alloc = (struct mblock_used*) ((uint32_t)found + found->free_size);
        #elif CPU_WORD_SIZE == 2
        alloc = (struct mblock_used*) ((uint16_t)found + found->free_size);
        #endif
    }

    // save the size of the allocated block
    alloc->size = totalsize;
    alloc->corrupt_check = KE_NOT_BUFF_CORRUPTED;

#if (LOG_MEM_USAGE)
    if (type == heap_id)
    {
        mem_log[heap_id].used_sz += totalsize;
        if (mem_log[heap_id].max_used_sz < mem_log[heap_id].used_sz)
            mem_log[heap_id].max_used_sz = mem_log[heap_id].used_sz;
    }
    else
    {
        mem_log[type].used_other_sz += totalsize;
        if (mem_log[type].max_used_other_sz < mem_log[type].used_other_sz)
            mem_log[type].max_used_other_sz = mem_log[type].used_other_sz;
    }

    alloc->size |= ((type & 0xF) << 12);
#endif
    
    // move to the user memory space
    alloc++;
    }

    // end of protection (as early as possible)
    GLOBAL_INT_RESTORE();
    ASSERT_ERR(node == NULL);

    return (void*)alloc;
}


void log_ke_free(void* mem_ptr)
{
    struct mblock_free *freed;
    struct mblock_used *bfreed;
    struct mblock_free *node, *next_node, *prev_node;
    uint32_t size;
    uint8_t cursor = 0;

    // sanity checks
    ASSERT_ERR(mem_ptr != NULL);
    
    // point to the block descriptor (before user memory so decrement)
    bfreed = ((struct mblock_used *)mem_ptr) - 1;

    // check if memory block has been corrupted or not
    ASSERT_ERR(bfreed->corrupt_check == KE_NOT_BUFF_CORRUPTED);
    // change corruption tocken in order to know if buffer has been already freed.
    bfreed->corrupt_check = KE_NOT_BUFF_FREE_CORRUPTED;

    // point to the first node of the free elements linked list
    node = NULL;

    freed = ((struct mblock_free *)bfreed);

    // protect accesses to descriptors
    GLOBAL_INT_DISABLE();

    // Retrieve where memory block comes from
    while(((cursor < KE_MEM_BLOCK_MAX)) && (node == NULL)){

        if(ke_mem_is_in_heap(cursor, mem_ptr))
        {
            // Select Heap to use, first try to use current heap.
            node = ke_env.heap[cursor];
        }
        else
        {
            cursor ++;
        }
    }

#if (LOG_MEM_USAGE)
    uint16_t heap_id;

    heap_id = ((bfreed->size & 0xF000) >> 12);
    bfreed->size &= 0xFFF;

    if (cursor == heap_id)
    {
        mem_log[heap_id].used_sz -= bfreed->size;
    }
    else
    {
        mem_log[heap_id].used_other_sz -= bfreed->size;
    }
#endif
    size = bfreed->size;

    // sanity checks
    ASSERT_ERR(node != NULL);
    ASSERT_ERR(((uint32_t)mem_ptr > (uint32_t)node));

    prev_node = NULL;

    while(node != NULL)
    {
        ASSERT_ERR(node->corrupt_check == KE_NOT_LIST_CORRUPTED);
        // check if the freed block is right after the current block
        if ((uint32_t)freed == ((uint32_t)node + node->free_size))
        {
            // append the freed block to the current one
            node->free_size += size;

            // check if this merge made the link between free blocks
            if (((uint32_t) node->next) == (((uint32_t)node) + node->free_size))
            {
                next_node = node->next;
                // add the size of the next node to the current node
                node->free_size += next_node->free_size;
                // update the next of the current node
                ASSERT_ERR(next_node != NULL);
                node->next = next_node->next;
                // update linked list.
                if(next_node->next != NULL)
                {
                    next_node->next->previous = node;
                }
            }
            goto free_end;
        }
        else if ((uint32_t)freed < (uint32_t)node)
        {
            // sanity check: can not happen before first node
            ASSERT_ERR(prev_node != NULL);

            // update the next pointer of the previous node
            prev_node->next = freed;
            freed->previous = prev_node;

            freed->corrupt_check = KE_NOT_LIST_CORRUPTED;

            // check if the released node is right before the free block
            if (((uint32_t)freed + size) == (uint32_t)node)
            {
                // merge the two nodes
                freed->next = node->next;
                if(node->next != NULL)
                {
                    node->next->previous = freed;
                }
                freed->free_size = node->free_size + size;
            }
            else
            {
                // insert the new node
                freed->next = node;
                node->previous = freed;
                freed->free_size = size;
            }
            goto free_end;
        }

        // move to the next free block node
        prev_node = node;
        node = node->next;

    }

    // if reached here, freed block is after last free block and not contiguous
    prev_node->next = (struct mblock_free*)freed;
    freed->next = NULL;
    freed->previous = prev_node;
    freed->free_size = size;
    freed->corrupt_check = KE_NOT_LIST_CORRUPTED;


free_end:
    #if KE_PROFILING
    ke_env.heap_used[cursor] -= size;
    #endif //KE_PROFILING
    // end of protection
    GLOBAL_INT_RESTORE();
}

///@} KE_MEM
