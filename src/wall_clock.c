/**
 * @file:   wall_clock.c
 * @brief:  establish the wall clock process
 * @author: bsankara
 * @date:   2016/01/27
 */

#include "wall_clock.h"
#include "rtx.h"

PROC_INIT get_wall_clock_init(void) {
	PROC_INIT wall_clock_init;

	wall_clock_init.m_pid = PID_WALL_CLOCK;
	wall_clock_init.m_priority = MEDIUM;
	wall_clock_init.m_stack_size = 0x100;
	wall_clock_init.mpf_start_pc = &wall_clock;

	return wall_clock_init;
}

void wall_clock(void) {
	int hours = 0;
	int minutes = 0;
	int seconds = 0;
	int sender_id;
	int running = 0;

	// register self to kcd
	struct msgbuf *p_msg_env = (struct msgbuf*) request_memory_block();
	p_msg_env->mtype = KCD_REG;
	p_msg_env->mtext[0] = '%';
	p_msg_env->mtext[1] = 'W';
	p_msg_env->mtext[2] = '\0';
	send_message(PID_KCD, (void *)p_msg_env);


	while (1) {
		MSGBUF* p_envelope = receive_message(&sender_id);

		switch(sender_id) {
			case PID_KCD: {
				struct msgbuf *incrementTime;
				// p_envelope->mtext[0] will be % , mtext[1] will be W and the third will be the switch case
				char switchCase = p_envelope->mtext[2];
				if (switchCase == 'R') {
					hours = 0;
					minutes = 0;
					seconds = 0;

					release_memory_block(p_envelope);
					if (!running) {
						incrementTime =  (struct msgbuf*) request_memory_block();
						incrementTime->mtype = DEFAULT;
						delayed_send(PID_WALL_CLOCK, incrementTime, 1000);
					}

					running = 1;
				} else if (switchCase == 'S') {
					// mtext[3] is a space, mtext[4] is the tens of hours and so on
					hours = (p_envelope->mtext[4] - '0') *10;
					hours = hours + (p_envelope->mtext[5] - '0');
					minutes = (p_envelope->mtext[7] - '0') * 10;
					minutes = minutes + (p_envelope->mtext[8] - '0');
					seconds = (p_envelope->mtext[10] - '0') * 10;
					seconds = seconds + (p_envelope->mtext[11] - '0');
					release_memory_block(p_envelope);

					if (!running) {
						incrementTime =  (struct msgbuf*) request_memory_block();
						incrementTime->mtype = DEFAULT;
						delayed_send(PID_WALL_CLOCK, incrementTime, 1000);
					}

					running = 1;
				} else if (switchCase == 'T') {
					running = 0;
					release_memory_block(p_envelope);
				}
				break;
			}

			case PID_WALL_CLOCK: {
				struct msgbuf *printMsg;
				struct msgbuf *incrementTime;
				if (running) {
					// increment the time
					if (seconds < 59) {
						seconds++;
					}
					else if (minutes < 59) {
						minutes++;
						seconds = 0;
					}
					else if (hours < 23) {
						hours++;
						minutes = 0;
						seconds = 0;
					} else {
						hours = 0;
						minutes = 0;
						seconds = 0;
					}

					printMsg = (struct msgbuf*) request_memory_block();
					printMsg->mtype = DEFAULT;
					printMsg->mtext[0] = (hours /10) + '0';
					printMsg->mtext[1] = (hours % 10) + '0';
					printMsg->mtext[2] = ':';
					printMsg->mtext[3] = (minutes /10) + '0';
					printMsg->mtext[4] = (minutes % 10) + '0';
					printMsg->mtext[5] = ':';
					printMsg->mtext[6] = (seconds /10) + '0';
					printMsg->mtext[7] = (seconds % 10) + '0';
					printMsg->mtext[8] = '\r';
					printMsg->mtext[9] = '\n';
					printMsg->mtext[10] = 0;
					send_message(PID_CRT, (void *) printMsg);

					incrementTime =  (struct msgbuf*) request_memory_block();
					incrementTime->mtype = DEFAULT;
					delayed_send(PID_WALL_CLOCK, incrementTime, 1000);
					release_memory_block(p_envelope);
				}
			}
			default: {
				release_memory_block(p_envelope);
			}
		}
	}
}
