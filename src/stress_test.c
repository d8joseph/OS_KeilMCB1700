/**
 * @file:   stress_test.c
 * @brief:  establish the stress test process
 * @author: bsankara
 * @date:   2016/03/20
 */

#include "stress_test.h"
#include "rtx.h"

PROC_INIT get_process_a_init(void) {
	PROC_INIT proc_a_init;

	proc_a_init.m_pid = PID_A;
	proc_a_init.m_priority = LOW;
	proc_a_init.m_stack_size = 0x200;
	proc_a_init.mpf_start_pc = &stress_process_a;

	return proc_a_init;
}

PROC_INIT get_process_b_init(void) {
	PROC_INIT proc_b_init;

	proc_b_init.m_pid = PID_B;
	proc_b_init.m_priority = LOW;
	proc_b_init.m_stack_size = 0x200;
	proc_b_init.mpf_start_pc = &stress_process_b;

	return proc_b_init;
}

PROC_INIT get_process_c_init(void) {
	PROC_INIT proc_c_init;

	proc_c_init.m_pid = PID_C;
	proc_c_init.m_priority = MEDIUM;
	proc_c_init.m_stack_size = 0x200;
	proc_c_init.mpf_start_pc = &stress_process_c;

	return proc_c_init;
}

void stress_process_a(void) {
	// register self to kcd
	struct msgbuf *p_msg_env = (struct msgbuf*) request_memory_block();
	int num = 0;
	p_msg_env->mtype = KCD_REG;
	p_msg_env->mtext[0] = '%';
	p_msg_env->mtext[1] = 'Z';
	p_msg_env->mtext[2] = '\0';
	send_message(PID_KCD, (void *)p_msg_env);


	while(1) {
		p_msg_env = receive_message(NULL);
		if (p_msg_env->mtext[0] == '%' && p_msg_env->mtext[1] == 'Z') {
			release_memory_block(p_msg_env);
			break;
		} else {
			release_memory_block(p_msg_env);
		}
	}

	while(1) {
		p_msg_env = (struct msgbuf*) request_memory_block();
		p_msg_env->mtype = COUNT_REPORT;
		*((int*) p_msg_env->mtext) = num;
		send_message(PID_B, p_msg_env);
		num = num + 1;
		release_processor();
	}
}
void stress_process_b(void) {
	while(1) {
		struct msgbuf *p_msg_env = receive_message(NULL);
		send_message(PID_C, p_msg_env);
	}
}
void stress_process_c(void) {

	char* proc_c_msg = "Process C\r\n";\

	typedef struct queue_block
	{
		int m_type;
		int m_count;
		struct queue_block* mp_next;
	} QUEUE_BLOCK;

	QUEUE_BLOCK *mp_first_queued_message = NULL;
	QUEUE_BLOCK *mp_last_queued_message = NULL;
	struct msgbuf *p_msg_env;
	struct msgbuf *q_msg_env;
	while(1)
	{
		if (mp_first_queued_message == NULL) {
			p_msg_env = receive_message(NULL);
		}
		else {
			p_msg_env = (struct msgbuf*) mp_first_queued_message;
			mp_first_queued_message = mp_first_queued_message->mp_next;
		}

		if (p_msg_env->mtype == COUNT_REPORT) {
			if (p_msg_env->mtext[0] % 20 == 0) {
				char* s = proc_c_msg;
				int i = 0;

				while (*s) {
					p_msg_env->mtext[i++] = *s++;
				}
				p_msg_env->mtext[i++] = 0;

				send_message(PID_CRT, p_msg_env);

				q_msg_env = (struct msgbuf*)request_memory_block();
				q_msg_env->mtype = WAKEUP10;
				delayed_send(PID_C, q_msg_env, 10);
				while (1) {
					p_msg_env = receive_message(NULL);
					if (p_msg_env->mtype == WAKEUP10) {
						// Note: not releasing leaks memory but the spec doesn't say to release.
						// release_memory_block(p_msg_env);
						break;
					}
					else {
						// queue the message
						if (mp_first_queued_message == NULL) {
							mp_first_queued_message = (QUEUE_BLOCK*) p_msg_env;
							mp_last_queued_message = (QUEUE_BLOCK*) p_msg_env;
						} else {
							mp_last_queued_message->mp_next = (QUEUE_BLOCK*) p_msg_env;
							((QUEUE_BLOCK*) p_msg_env)->mp_next = NULL;
							mp_last_queued_message = (QUEUE_BLOCK*) p_msg_env;
						}
					}
				}
			}
		}
		release_memory_block(p_msg_env);
		release_processor();
	}
}
