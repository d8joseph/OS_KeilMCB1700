/**
 * @file:   k_rtx_init.c
 * @brief:  Kernel initialization C file
 * @auther: Yiqing Huang
 * @date:   2014/01/17
 */

#include "k_rtx_init.h"
#include "uart_polling.h"
#include "uart.h"
#include "timer.h"
#include "k_memory.h"
#include "k_process.h"
#include "usr_proc.h"

void k_rtx_init(void)
{
	uart0_irq_init();   // uart0, interrupt-driven
	uart1_init();       // uart1, polling
	// THIS IS SET TO 1 WHILE TIMING
	timer_init(0);
	pre_process_init();
	memory_init();
	process_init();

	/* start the first process */
	k_release_processor();
}
