/**
 * @file:   k_process.h
 * @brief:  process management hearder file
 * @author: Yiqing Huang
 * @author: Thomas Reidemeister
 * @date:   2014/01/17
 * NOTE: Assuming there are only two user processes in the system
 */

#ifndef K_PROCESS_H_
#define K_PROCESS_H_

#include "k_rtx.h"

/* ----- Definitions ----- */

#define INITIAL_xPSR 0x01000000        /* user process initial xPSR value */

/* ----- Functions ----- */

int check_preemption(void);            /* preempt if the currently running process is too low of priority */
int queue_proc(PCB* pcb);              /* queue a proc that just became unblocked */

void pre_process_init(void);           /* collects process init info into one array */
void process_init(void);               /* initialize all procs in the system */
PCB *scheduler(void);                  /* pick the pid of the next to run process */
int k_release_processor(void);         /* kernel release_processor function */
int k_get_pid(void);                   /* return the process id of the currently running process */
int k_get_process_priority(int pid);   /* kernel get_process_priority function */
int k_set_process_priority(int pid, int priority);   /* kernel set_process_priority function */

extern void __rte(void);               /* pop exception stack frame */
extern void set_test_procs(void);      /* test process initial set up */

#endif /* ! K_PROCESS_H_ */
