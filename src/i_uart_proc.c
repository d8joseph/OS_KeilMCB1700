/**
 * @file:   i_uart_proc.h
 * @brief:  establish the i-uart process
 * @author: pwrobel
 * @date:   2016/03/06
 */

#include "i_uart_proc.h"


PROC_INIT get_i_uart_proc_init(void) {
  PROC_INIT i_uart_proc_init;

  i_uart_proc_init.m_pid = PID_I_UART;
  i_uart_proc_init.m_priority = HIGH;
  i_uart_proc_init.m_stack_size = 0x0;
  i_uart_proc_init.mpf_start_pc = NULL; // process is superficial and does not run

  return i_uart_proc_init;
}
