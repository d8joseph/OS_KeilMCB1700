/* @brief: common.h, definitions common to user dev and kernel dev
 * @author: pwrobel
 * @date: 2016/03/13
 */

#ifndef COMMON_H_
#define COMMON_H_

/* Process ID definitions */
#define PID_NULL 0
#define PID_A 7
#define PID_B 8
#define PID_C 9
#define PID_SET_PRIORITY 10
#define PID_WALL_CLOCK 11
#define PID_KCD 12
#define PID_CRT 13
#define PID_I_TIMER 14
#define PID_I_UART 15


typedef unsigned int U32;

/* initialization table item */
typedef struct proc_init
{
  int m_pid;              /* process id */
  int m_priority;         /* initial priority, not used in this example. */
  int m_stack_size;       /* size of stack in words */
  void (*mpf_start_pc) ();/* entry point of the process */
} PROC_INIT;

/* message envelop for IPC */
typedef struct msgbuf {
  int mtype;
  char mtext[1];
} MSGBUF;

#endif
