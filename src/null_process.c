/**
 * @file:   null_process.h
 * @brief:  establish the null process
 * @author: pwrobel
 * @date:   2016/01/27
 */

#include "null_process.h"
#include "rtx.h"

PROC_INIT get_null_proc_init(void) {
  PROC_INIT null_proc_init;

  null_proc_init.m_pid = PID_NULL;
  null_proc_init.m_priority = LOWEST + 1;
  null_proc_init.m_stack_size = 0x100;
  null_proc_init.mpf_start_pc = &null_proc;

  return null_proc_init;
}

void null_proc(void) {
  while (1) {
    release_processor();
  }
}
