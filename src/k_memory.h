/**
 * @file:   k_memory.h
 * @brief:  kernel memory managment header file
 * @author: Yiqing Huang
 * @date:   2014/01/17
 */

#ifndef K_MEM_H_
#define K_MEM_H_

#include "k_rtx.h"

/* ----- Definitions ----- */
#define RAM_END_ADDR 0x10008000
#define MAX_MEMORY_BLOCKS 5000

typedef struct memory_list
{
  MEMORY_BLOCK* mp_start;
  int m_blocks_left;
} MEMORY_LIST;

/* ----- Functions ------ */
void memory_init(void);
U32 *alloc_stack(U32 size_b);
void *k_request_memory_block(void);
void *k_request_memory_block_custom(int is_blocking);
int k_release_memory_block(void *);

// Initializes the memory. Must call this
// to get a valid MemoryList pointer.
void mem_list_init(void);

// Frees a block of memory back to the memory list.
// Adds a block with the given address at the front of the given MemoryList.
int free_memory(MEMORY_BLOCK*);

// Gets a block of memory from the memory list.
// Removes a block from the front of the given MemoryList and return
// its address.
MEMORY_BLOCK* get_memory(void);

#endif /* ! K_MEM_H_ */
