/**
 * @file:   KCD.c
 * @brief:  KCD implementation
 * @author: d8joseph
 * @date:   2016/03/04
 */

#include "KCD.h"
#include "rtx.h"
#include <LPC17xx.h>
#include "uart.h"
#include "uart_polling.h"


typedef struct pidList {
	int pid;
	char keyWord;
} PID_LIST;

extern PCB** gp_pcbs;
extern PROC_INIT* g_proc_table;

PROC_INIT get_KCD_init(void) {
	PROC_INIT KCD_init;

	KCD_init.m_pid = PID_KCD;
	KCD_init.m_priority = HIGH;
	KCD_init.m_stack_size = 0x600;
	KCD_init.mpf_start_pc = &KCD;

	return KCD_init;
}

void KCD(void) {
	int PID_COUNTER =0;
	int sender_id;
	char buffer[64];
	char *s = buffer;
	PID_LIST mappingList[100];
	while (1) {
		MSGBUF* p_envelope = receive_message(&sender_id);

		if(p_envelope->mtype == KCD_REG){
			//register KCD
			if (PID_COUNTER < 100) {
				mappingList[PID_COUNTER].pid = sender_id;
				mappingList[PID_COUNTER].keyWord = p_envelope->mtext[1];
				PID_COUNTER++;
			}
			release_memory_block(p_envelope);

		}
		else{
			MSGBUF* outMsg = (MSGBUF*) request_memory_block();
			outMsg->mtext[0] = p_envelope->mtext[0];
			outMsg->mtext[1] = 0;
			send_message(PID_CRT, outMsg);

			// first character and it is a %
			if (p_envelope->mtext[0] != '\r') {
				if (s-buffer < 63) {
					*(s++) = p_envelope->mtext[0];
				}
			} else {
				outMsg = (MSGBUF*) request_memory_block();
				outMsg->mtext[0] = '\n';
				outMsg->mtext[1] = 0;
				send_message(PID_CRT, outMsg);
				*(s++) = 0;
				if (s - buffer >= 3 && buffer[0] == '%') {
					int i = 0;
					for (i = 0; i < PID_COUNTER; i++) {
						// we found the proc to send the things to
						if (mappingList[i].keyWord == buffer[1]) {
							int j = 0;
							MSGBUF* sendMessage = (MSGBUF*) request_memory_block();
							sendMessage->mtype = DEFAULT;
							s = buffer;
							while (*s) {
								sendMessage->mtext[j++] = *(s++);
							}
							sendMessage->mtext[j++] = 0;
							send_message(mappingList[i].pid, sendMessage);
						}
					}
				}
				s = buffer;
			}
			release_memory_block(p_envelope);
		}
	}
}
