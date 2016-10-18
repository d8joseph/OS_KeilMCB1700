/**
 * @file:   CRT.c
 * @brief:  establish the CRT process
 * @author: bsankara
 * @date:   2016/03/04
 */

#include "CRT.h"
#include "rtx.h"
#include <LPC17xx.h>
#include "uart.h"

PROC_INIT get_CRT_init(void) {
	PROC_INIT CRT_init;

	CRT_init.m_pid = PID_CRT;
	CRT_init.m_priority = HIGH;
	CRT_init.m_stack_size = 0x100;
	CRT_init.mpf_start_pc = &CRT;

	return CRT_init;
}

void CRT(void) {
	MEMORY_BLOCK *mp_first_queued_message = NULL;
	MEMORY_BLOCK *mp_last_queued_message = NULL;
	int sender_id;
	MEMORY_BLOCK* p_msg_block;
	MSGBUF* curr_block;
	LPC_UART_TypeDef *pUart;
	char* curChar;
	while (1) {
		MSGBUF* p_envelope = receive_message(&sender_id);
		p_msg_block = (MEMORY_BLOCK*) ((U8*) p_envelope - sizeof(MEMORY_BLOCK_HEADER));
		if (sender_id == PID_I_UART) {
			if (*curChar != '\0') {
				pUart = (LPC_UART_TypeDef *)LPC_UART0;
				pUart->IER = IER_THRE | IER_RLS | IER_RBR;
				pUart->THR = *curChar;
				curChar++;
			}
			else {
				p_msg_block = mp_first_queued_message;
				mp_first_queued_message = mp_first_queued_message->m_header.mp_next_block;

				release_memory_block(p_msg_block->mp_shared_mem);
				if(mp_first_queued_message != NULL) {
					curr_block = (MSGBUF*)mp_first_queued_message->mp_shared_mem;
					curChar = curr_block->mtext;
					pUart = (LPC_UART_TypeDef *)LPC_UART0;
					pUart->IER = IER_THRE | IER_RLS | IER_RBR;
					pUart->THR = *curChar;
					curChar++;
				}
			}
		}
		else {
			if (mp_first_queued_message == NULL) {
				mp_first_queued_message = p_msg_block;
				mp_last_queued_message = p_msg_block;
				curChar = p_envelope->mtext;
				pUart = (LPC_UART_TypeDef *)LPC_UART0;
				pUart->IER = IER_THRE | IER_RLS | IER_RBR;
				pUart->THR = *curChar;
				curChar++;
			}
			else {
				mp_last_queued_message->m_header.mp_next_block = p_msg_block;
				mp_last_queued_message = p_msg_block;
			}
		}
	}
}
