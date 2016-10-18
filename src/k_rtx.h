/**
 * @file:   k_rtx.h
 * @brief:  kernel deinitiation and data structure header file
 * @auther: Yiqing Huang
 * @date:   2014/01/17
 */

#ifndef K_RTX_H_
#define K_RTX_H_

#include "common.h"

/*----- Definitions -----*/

#define RTX_ERR -1
#define RTX_OK  0

#define NULL 0
#define NUM_TEST_PROCS 6
#define NUM_TOTAL_PROCS 16
#define PRIORITY_LEVELS 4

#define MEMORY_BLOCK_SIZE 0x80

#define TIMER_SIGNAL_STATIC_MEM 1

/* Process Priority. The bigger the number is, the lower the priority is*/
#define HIGH    0
#define MEDIUM  1
#define LOW     2
#define LOWEST  3

/* Message passing defines */
#define DEFAULT 0
#define KCD_REG 1

/*----- Types -----*/
typedef unsigned char U8;
typedef unsigned int U32;

/* Process states */
typedef enum {NEW = 0, RDY, RUN, BLOCKED_BY_MEM, WAITING_ON_MSG} PROC_STATE_E;

/*
  Memory block definitions. These define how dynamic memory on the heap is managed.
*/
struct pcb;
struct memory_block;

typedef struct memory_block_header
{
  struct memory_block* mp_next_block;
  struct pcb* mp_owner;
  struct pcb* mp_message_recipient;
  struct pcb* mp_message_sender;
  int m_count;  // general purpose counter
} MEMORY_BLOCK_HEADER;

typedef struct memory_block
{
  MEMORY_BLOCK_HEADER m_header;
  U8 mp_shared_mem[MEMORY_BLOCK_SIZE - sizeof(MEMORY_BLOCK_HEADER)];
} MEMORY_BLOCK;

/*
  PCB data structure definition.
  You may want to add your own member variables
  in order to finish P1 and the entire project
*/
typedef struct pcb
{
  U32 *mp_sp;   /* stack pointer of the process */
  U32 m_pid;    /* process id */
  PROC_STATE_E m_state;   /* state of the process */
  U32 m_priority;         /* process priority */
  MEMORY_BLOCK *mp_first_queued_message;
  MEMORY_BLOCK *mp_last_queued_message;
  MEMORY_BLOCK **mp_future_message;
} PCB;


#endif // ! K_RTX_H_
