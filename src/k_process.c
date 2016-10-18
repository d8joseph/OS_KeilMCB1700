/**
 * @file:   k_process.c
 * @brief:  process management C file
 * @author: Yiqing Huang
 * @author: Thomas Reidemeister
 * @date:   2014/02/28
 * NOTE: The example code shows one way of implementing context switching.
 *       The code only has minimal sanity check. There is no stack overflow check.
 *       The implementation assumes NO HARDWARE INTERRUPTS.
 *       The purpose is to show how context switch could be done under stated assumptions.
 *       These assumptions are not true in the required RTX Project!!!
 *       If you decide to use this piece of code, you need to understand the assumptions and
 *       the limitations.
 */

#include <LPC17xx.h>
#include <system_LPC17xx.h>
#include "k_process.h"
#include "k_memory.h"
#include "queue.h"
#include "min_heap.h"

#ifdef DEBUG_0
#include "printf.h"
#endif

/* ----- Global Variables ----- */
PCB **gp_pcbs;                  /* array of pcbs */
PCB *gp_current_process = NULL; /* always point to the current RUN process */

/* process initialization table */
PROC_INIT g_proc_table[NUM_TOTAL_PROCS];
extern PROC_INIT g_test_procs[NUM_TEST_PROCS];

/* queues for processes waiting to run */
QUEUE gp_ready_queues[PRIORITY_LEVELS];

extern MIN_HEAP g_blocked_by_memory_heap;

int g_preempt = 0;

/* extern proc init functions */
extern PROC_INIT get_null_proc_init(void);
extern PROC_INIT get_CRT_init(void);
extern PROC_INIT get_i_timer_proc_init(void);
extern PROC_INIT get_i_uart_proc_init(void);
extern PROC_INIT get_wall_clock_init(void);
extern PROC_INIT get_KCD_init(void);
extern PROC_INIT get_set_priority_init(void);
extern PROC_INIT get_process_a_init(void);
extern PROC_INIT get_process_b_init(void);
extern PROC_INIT get_process_c_init(void);
/**
 * @brief: Coalesce the proc initialization values for the various procs on the system.
 */
void pre_process_init()
{
  int i;

  set_test_procs();

  /**
   * Fill out the initialization table with defaults since we don't
   * have to implement all the processes yet
   */
  for (i = 0; i < NUM_TOTAL_PROCS; i++) {
    g_proc_table[i].m_pid = i;
    g_proc_table[i].m_priority = LOWEST;
    g_proc_table[i].m_stack_size = 0x0;
    g_proc_table[i].mpf_start_pc = NULL;
  }

  g_proc_table[PID_NULL] = get_null_proc_init();
  g_proc_table[PID_I_TIMER] = get_i_timer_proc_init();
  g_proc_table[PID_I_UART] = get_i_uart_proc_init();
  g_proc_table[PID_CRT] = get_CRT_init();
  g_proc_table[PID_WALL_CLOCK] = get_wall_clock_init();
  g_proc_table[PID_KCD] = get_KCD_init();
  g_proc_table[PID_SET_PRIORITY] = get_set_priority_init();
	g_proc_table[PID_A] = get_process_a_init();
	g_proc_table[PID_B] = get_process_b_init();
	g_proc_table[PID_C] = get_process_c_init();

  for (i = 0; i < NUM_TEST_PROCS; i++) {
    g_proc_table[i + 1].m_pid = g_test_procs[i].m_pid;
    g_proc_table[i + 1].m_priority = g_test_procs[i].m_priority;
    g_proc_table[i + 1].m_stack_size = g_test_procs[i].m_stack_size;
    g_proc_table[i + 1].mpf_start_pc = g_test_procs[i].mpf_start_pc;
  }
}

/**
 * @brief: initialize all processes in the system
 * NOTE: Must be called after memory_init()
 */
void process_init()
{
  int i;
  U32 *sp;

  /* initialize waiting queues */
  for (i = 0; i < PRIORITY_LEVELS; i++) {
    queue_init(&gp_ready_queues[i]);
  }

  /* initialize exception stack frame (i.e. initial context) for each process */
  for (i = 0; i < NUM_TOTAL_PROCS; i++) {
    gp_pcbs[i]->m_pid = g_proc_table[i].m_pid;
    gp_pcbs[i]->m_priority = g_proc_table[i].m_priority;
    gp_pcbs[i]->m_state = NEW;
    gp_pcbs[i]->mp_sp = NULL;
    gp_pcbs[i]->mp_first_queued_message = NULL;
    gp_pcbs[i]->mp_last_queued_message = NULL;

    if (g_proc_table[i].mpf_start_pc != NULL) {
      int j;

      queue_add(&gp_ready_queues[gp_pcbs[i]->m_priority], gp_pcbs[i]);

      sp = alloc_stack(g_proc_table[i].m_stack_size);
      *(--sp) = INITIAL_xPSR;      // user process initial xPSR
      *(--sp) = (U32)(g_proc_table[i].mpf_start_pc); // PC contains the entry point of the process
      for (j = 0; j < 6; j++) { // R0-R3, R12 are cleared with 0
        *(--sp) = 0x0;
      }
      gp_pcbs[i]->mp_sp = sp;
    }
  }
}

/*@brief: scheduler, pick the pid of the next to run process
 *@return: PCB pointer of the next to run process
 *         NULL if error happens
 *POST: if gp_current_process was NULL, then it gets set to pcbs[0].
 *      No other effect on other global variables.
 */
PCB *scheduler(void)
{
  int i;
  if (gp_current_process != NULL) {
    if (gp_current_process != gp_pcbs[PID_NULL] && gp_current_process->m_state != BLOCKED_BY_MEM && gp_current_process->m_state != WAITING_ON_MSG) {
      if (g_preempt) {
        queue_add_front(&gp_ready_queues[gp_current_process->m_priority], gp_current_process);
        g_preempt = 0;
      } else {
        queue_add(&gp_ready_queues[gp_current_process->m_priority], gp_current_process);
      }
      gp_current_process->m_state = RDY;
    }
  }

  for (i = 0; i < PRIORITY_LEVELS; i++) {
    if (!queue_empty(&gp_ready_queues[i])) {
      return (PCB*) queue_next(&gp_ready_queues[i]);
    }
  }

  return gp_pcbs[PID_NULL];
}

/*@brief: switch out old pcb (p_pcb_old), run the new pcb (gp_current_process)
 *@param: p_pcb_old, the old pcb that was in RUN
 *@return: RTX_OK upon success
 *         RTX_ERR upon failure
 *PRE:  p_pcb_old and gp_current_process are pointing to valid PCBs.
 *POST: if gp_current_process was NULL, then it gets set to pcbs[0].
 *      No other effect on other global variables.
 */
int process_switch(PCB *p_pcb_old)
{
  PROC_STATE_E state;

  state = gp_current_process->m_state;

  if (state == NEW) {
    if (gp_current_process != p_pcb_old && p_pcb_old->m_state == RUN) {
      p_pcb_old->m_state = RDY;
    }

    if (gp_current_process != p_pcb_old && p_pcb_old->m_state != NEW) {
      p_pcb_old->mp_sp = (U32 *) __get_MSP();
    }
    gp_current_process->m_state = RUN;
    __set_MSP((U32) gp_current_process->mp_sp);
    __rte();  // pop exception stack frame from the stack for a new processes
  }

  /* The following will only execute if the if block above is FALSE */
  if (gp_current_process != p_pcb_old) {
    if (state == RDY){
      p_pcb_old->mp_sp = (U32 *) __get_MSP(); // save the old process's sp
      gp_current_process->m_state = RUN;
      __set_MSP((U32) gp_current_process->mp_sp); //switch to the new proc's stack
    } else {
      gp_current_process = p_pcb_old; // revert back to the old proc on error
      return RTX_ERR;
    }
  }

  return RTX_OK;
}

/**
 * @brief release_processor().
 * @return RTX_ERR on error and zero on success
 * POST: gp_current_process gets updated to next to run process
 */
int k_release_processor(void)
{
  PCB *p_pcb_old = NULL;

  p_pcb_old = gp_current_process;
  gp_current_process = scheduler();

  if (gp_current_process == NULL) {
    gp_current_process = p_pcb_old; // revert back to the old process
    return RTX_ERR;
  }

  if (p_pcb_old == NULL) {
    p_pcb_old = gp_current_process;
  }

  return process_switch(p_pcb_old);
}

int k_get_pid()
{
  if (gp_current_process == NULL) {
    #ifdef DEBUG_0
      printf("Call to get_pid when no process is running\r\n");
    #endif
    return 0;
  }

  return gp_current_process->m_pid;
}

int k_get_process_priority(int pid)
{
  if (pid >= 0 && pid < NUM_TOTAL_PROCS) {
    return gp_pcbs[pid]->m_priority;
  } else {
    return RTX_ERR;
  }
}

int k_set_process_priority(int pid, int priority)
{
  if (
    (pid >= 1 && pid <= NUM_TEST_PROCS) ||
    (pid == PID_WALL_CLOCK) ||
    (pid == PID_SET_PRIORITY) ||
    (pid == PID_A) ||
    (pid == PID_B) ||
    (pid == PID_C)
  ) {
    if (priority < HIGH || priority > LOWEST) {
      return RTX_ERR;
    } else {
      PCB* p_pcb = gp_pcbs[pid];

      if (p_pcb->m_priority == priority) {
        return RTX_OK;
      }

      switch (p_pcb->m_state) {
        case BLOCKED_BY_MEM:
        {
          min_heap_remove(&g_blocked_by_memory_heap, p_pcb);
          p_pcb->m_priority = priority;
          min_heap_add(&g_blocked_by_memory_heap, p_pcb);
          return RTX_OK;
        }
        case RUN:
        {
          p_pcb->m_priority = priority;
          return check_preemption();
        }
        case RDY:
        case NEW:
        {
          queue_remove(&gp_ready_queues[p_pcb->m_priority], p_pcb);
          p_pcb->m_priority = priority;
          queue_add(&gp_ready_queues[p_pcb->m_priority], p_pcb);
          return check_preemption();
        }
        case WAITING_ON_MSG:
        {
          p_pcb->m_priority = priority;
          return RTX_OK;
        }
      }
    }
  }

  return RTX_ERR;
}

int check_preemption(void) {
  if (gp_current_process == NULL) {
    return k_release_processor();
  } else {
    int i;
    int priority = gp_current_process->m_priority;
    int preempted = 0;
    for (i = HIGH; i < priority; i++) {
      if (!queue_empty(&gp_ready_queues[i])) {
        preempted = 1;
        break;
      }
    }

    if (preempted) {
      g_preempt = 1;
      return k_release_processor();
    }
  }

  return RTX_OK;
}

int queue_proc(PCB* pcb) {
  if (pcb->m_state != BLOCKED_BY_MEM && pcb->m_state != WAITING_ON_MSG) {
    return RTX_OK;
  }

  pcb->m_state = RDY;
  queue_add(&gp_ready_queues[pcb->m_priority], pcb);
  return check_preemption();
}
