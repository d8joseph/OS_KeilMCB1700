/**
 * @file:   k_memory.c
 * @brief:  kernel memory managment routines
 * @author: Yiqing Huang
 * @date:   2014/01/17
 */

#include "k_memory.h"
#include "k_process.h"
#include "min_heap.h"

#ifdef DEBUG_0
#include "printf.h"
#endif

MIN_HEAP g_blocked_by_memory_heap;

typedef struct blocked_by_memory_data {
  void* m_returned_memory;
  PCB* m_pcb;
} BLOCKED_BY_MEMORY_DATA;

int blocked_memory_predicate(void* left, void* right) {
  return ((BLOCKED_BY_MEMORY_DATA*)left)->m_pcb->m_priority < ((BLOCKED_BY_MEMORY_DATA*)right)->m_pcb->m_priority;
}

/* ----- External Variables ----- */
extern unsigned int Image$$RW_IRAM1$$ZI$$Limit;
extern PCB **gp_pcbs;
extern PCB *gp_current_process;
extern PROC_INIT g_proc_table[NUM_TOTAL_PROCS];

/* ----- Global Variables ----- */
U32 *gp_stack; /* The last allocated stack low address. 8 bytes aligned */
         /* The first stack starts at the RAM high address */
         /* stack grows down. Fully decremental stack */
U8 *gp_start;
U32 *gp_end;

MEMORY_LIST g_mem_list;

/**
 * @brief: Initialize RAM as follows:

0x10008000+---------------------------+ High Address
      |    Proc 1 STACK           |
      |---------------------------|
      |    Proc 2 STACK           |
      |---------------------------|
      |         .                 |
      |         .                 |
      |         .                 |
      |---------------------------|<--- gp_stack
      |                           |
      |        HEAP               |
      |                           |
      |---------------------------|
      |         .                 |
      |         .                 |
      |         .                 |
      |---------------------------|
      |        PCB 2              |
      |---------------------------|
      |        PCB 1              |
      |---------------------------|
      |        PCB pointers       |
      |---------------------------|<--- gp_pcbs
      |        Padding            |
      |---------------------------|
      |Image$$RW_IRAM1$$ZI$$Limit |
      |...........................|
      |       RTX  Image          |
      |                           |
0x10000000+---------------------------+ Low Address

*/

void memory_init(void)
{
  int i;
  gp_start = (U8 *)&Image$$RW_IRAM1$$ZI$$Limit;

  /* 4 bytes padding */
  gp_start += 4;

  /* allocate memory for pcb pointers   */
  gp_pcbs = (PCB **)gp_start;
  gp_start += NUM_TOTAL_PROCS * sizeof(PCB *);

  for (i = 0; i < NUM_TOTAL_PROCS; i++) {
    gp_pcbs[i] = (PCB *)gp_start;
    gp_start += sizeof(PCB);
  }

  /* prepare for alloc_stack() to allocate memory for stacks */
  gp_stack = (U32 *)RAM_END_ADDR;
  if ((U32)gp_stack & 0x04) { /* 8 bytes alignment */
    --gp_stack;
  }

  gp_end = gp_stack;
  for (i = 0; i < NUM_TOTAL_PROCS; i++) {
    gp_end = (U32 *)((U8 *) gp_end - g_proc_table[i].m_stack_size);
    if ((U32) gp_end & 0x04) {
      gp_end--;
    }
  }

  min_heap_init(&g_blocked_by_memory_heap, &blocked_memory_predicate);
  mem_list_init();
}

/**
 * @brief: allocate stack for a process, align to 8 bytes boundary
 * @param: size, stack size in bytes
 * @return: The top of the stack (i.e. high address)
 * POST:  gp_stack is updated.
 */

U32 *alloc_stack(U32 size_b)
{
  U32 *sp;
  sp = gp_stack; /* gp_stack is always 8 bytes aligned */

  /* update gp_stack */
  gp_stack = (U32 *)((U8 *)sp - size_b);

  /* 8 bytes alignement adjustment to exception stack frame */
  if ((U32)gp_stack & 0x04) {
    --gp_stack;
  }
  return sp;
}

void *k_request_memory_block(void) {
  return k_request_memory_block_custom(1);
}

void *k_request_memory_block_custom(int is_blocking) {
  if (g_mem_list.mp_start == NULL) {
    void* p_future_mem;
    BLOCKED_BY_MEMORY_DATA bdata;

    #ifdef DEBUG_0
      printf("Out of memory!\n");
    #endif

    if (!is_blocking) {
      return NULL;
    }

    bdata.m_pcb = gp_current_process;
    bdata.m_returned_memory = &p_future_mem;
    min_heap_add(&g_blocked_by_memory_heap, &bdata);
    gp_current_process->m_state = BLOCKED_BY_MEM;
    k_release_processor();

    return p_future_mem;
  } else {
    MEMORY_BLOCK* p_block = g_mem_list.mp_start;
    g_mem_list.mp_start = p_block->m_header.mp_next_block;
    g_mem_list.m_blocks_left -= 1;

    p_block->m_header.mp_owner = gp_current_process;
    p_block->m_header.mp_next_block = NULL;

    #ifdef DEBUG_0
      printf("Allocating memory block of size %x at address %x \r\n", MEMORY_BLOCK_SIZE, p_block->mp_shared_mem);
    #endif

    return (void*) p_block->mp_shared_mem;
  }
}

int k_release_memory_block(void *p_mem) {
  MEMORY_BLOCK* p_mem_blk;
  p_mem_blk = (MEMORY_BLOCK*) ((U8*) p_mem - sizeof(MEMORY_BLOCK_HEADER));
  if (p_mem_blk->m_header.mp_owner != gp_current_process) {
    return RTX_ERR;
  }

  p_mem_blk->m_header.mp_owner = NULL;
  p_mem_blk->m_header.mp_next_block = g_mem_list.mp_start;
  g_mem_list.mp_start = p_mem_blk;
  g_mem_list.m_blocks_left += 1;

  if (!min_heap_empty(&g_blocked_by_memory_heap)) {
    BLOCKED_BY_MEMORY_DATA* bdata = min_heap_next(&g_blocked_by_memory_heap);
    *((void**) bdata->m_returned_memory) = k_request_memory_block();
    return queue_proc(bdata->m_pcb);
  }

  return RTX_OK;
}

// Initializes the memory and g_mem_list which manages said memory.
// Must call this before requesting memory.
void mem_list_init(void)
{
  /*
   * This function starts at gp_start and partitions
   * the memory space into MEMORY_BLOCK_SIZE blocks, up to
   * gp_end, and sets up g_mem_list that can be used along
   * with the add and remove functions to manage memory.
   *
   * The layout of a memory block that has size
   * MEMORY_BLOCK_SIZE is as follows:
   *
   *
   * ////////////////////////////////////////////
   * //                Header                  //
   * //                                        //
   * ////////////////////////////////////////////
   * //   Remaining block of shared memory     //
   * //                                        //
   * //                                        //
   * //                                        //
   * //                                        //
   * //                                        //
   * //                                        //
   * ////////////////////////////////////////////
   *
   * We can do this because if the memory is free, no one cares
   * if we use it for some internal book-keeping.
   *
   */

  MEMORY_BLOCK* p_curr_block;

  g_mem_list.mp_start = NULL;
  g_mem_list.m_blocks_left = 0;

  p_curr_block = (MEMORY_BLOCK*) gp_start;

  while ((p_curr_block + 1 <= (MEMORY_BLOCK*) gp_end) && g_mem_list.m_blocks_left < MAX_MEMORY_BLOCKS) {
    p_curr_block->m_header.mp_next_block = g_mem_list.mp_start;
    p_curr_block->m_header.mp_owner = NULL;
    p_curr_block->m_header.mp_message_recipient = NULL;
    p_curr_block->m_header.mp_message_sender = NULL;

    g_mem_list.mp_start = p_curr_block;
    g_mem_list.m_blocks_left += 1;

    p_curr_block += 1;
  }
}
