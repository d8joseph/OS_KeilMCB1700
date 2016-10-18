/**
 * @file:   usr_proc.c
 * @brief:  Some user processes
 * @author: Yiqing Huang
 * @date:   2014/02/28
 * NOTE: Each process is in an infinite loop. Processes never terminate.
 */

#include "rtx.h"
#include "uart_polling.h"
#include "usr_proc.h"
#include "LPC17xx.h"

#ifdef DEBUG_0
#include "printf.h"
#endif /* DEBUG_0 */

typedef enum {
  SIMPLE_PRINT = 0,
  MEMORY_STRESS,
  SEND_AND_RECEIVE,
  SEND_CRT_MESSAGES,
  SEND_DELAYED_MESSAGES,
  PREEMPTION,
  SET_PRIORITY,
	TIME_SEND,
	TIME_RECIEVE,
	TIME_REQUEST,
  NOTHING,
	
} TEST_CASE_E;

TEST_CASE_E g_test_case = TIME_REQUEST;

/* initialization table item */
PROC_INIT g_test_procs[NUM_TEST_PROCS];

void alphabet(void);
void numbers(void);
void fibonacci(void);
void mem_proc(void);
void priority_set(void);
void memory_hog(void);
void memory_user_priority_setter(void);
void nothing(void);
void sends_messages(void);
void receives_messages(void);
void sends_CRT_messages(void);
void sends_delayed_messages(void);
void print_then_release_proc_1(void);
void print_then_release_proc_2(void);
void set_priority_proc_1(void);
void set_priority_proc_2(void);
void delay(void);
void times_send_message(void);
void times_recieve_message(void);
void times_request_memory_block(void);

void set_test_procs() {
  int i;
  for (i = 0; i < NUM_TEST_PROCS; i++) {
    g_test_procs[i].m_pid = (U32)(i + 1);
    g_test_procs[i].m_stack_size = 0x200;
    g_test_procs[i].m_priority = LOWEST;
    g_test_procs[i].mpf_start_pc = &nothing;
  }

  switch (g_test_case) {
    // Should alternate between printing numbers and the alphabet
    case SIMPLE_PRINT:
      g_test_procs[0].m_priority = LOW;
      g_test_procs[0].mpf_start_pc = &alphabet;
      g_test_procs[1].m_priority = LOW;
      g_test_procs[1].mpf_start_pc = &numbers;
      break;

    // One process takes a little bit of memory, lowers its own priority
    // then another process tries to consume infinite memory but it blocks
    // until the first process releases its little bit of memory and then
    // the second process blocks again allowing the first one to continue
    // doing whatever
    case MEMORY_STRESS:
      g_test_procs[0].m_priority = HIGH;
      g_test_procs[0].mpf_start_pc = &memory_user_priority_setter;
      g_test_procs[1].m_priority = MEDIUM;
      g_test_procs[1].mpf_start_pc = &memory_hog;
      break;

    // One process should start by asking for a received message then be blocked.
    // This should then allow a lower priority process which sends messages to
    // other processes to run and then unblock the othe rprocess that is waiting.
    case SEND_AND_RECEIVE:
      g_test_procs[0].m_priority = MEDIUM;
      g_test_procs[0].mpf_start_pc = &receives_messages;
      g_test_procs[1].m_priority = LOW;
      g_test_procs[1].mpf_start_pc = &sends_messages;
      break;

    case SEND_CRT_MESSAGES:
      g_test_procs[0].m_priority = MEDIUM;
      g_test_procs[0].mpf_start_pc = &receives_messages;
      g_test_procs[1].m_priority = MEDIUM;
      g_test_procs[1].mpf_start_pc = &memory_hog;
      break;

    case SEND_DELAYED_MESSAGES:
      g_test_procs[0].m_priority = MEDIUM;
      g_test_procs[0].mpf_start_pc = &sends_delayed_messages;
      g_test_procs[0].m_stack_size = 0x800;
      break;

    case PREEMPTION:
      g_test_procs[0].m_priority = MEDIUM;
      g_test_procs[0].mpf_start_pc = &print_then_release_proc_1;
      g_test_procs[1].m_priority = MEDIUM;
      g_test_procs[1].mpf_start_pc = &print_then_release_proc_2;
      break;

		case SET_PRIORITY:
      g_test_procs[0].m_priority = MEDIUM;
      g_test_procs[0].mpf_start_pc = &set_priority_proc_1;
      g_test_procs[1].m_priority = LOW;
      g_test_procs[1].mpf_start_pc = &set_priority_proc_2;
			break;
		case TIME_SEND:
			g_test_procs[0].m_priority = HIGH;
      g_test_procs[0].mpf_start_pc = &times_send_message;
			break;
		case TIME_RECIEVE:
			g_test_procs[0].m_priority = HIGH;
      g_test_procs[0].mpf_start_pc = &times_recieve_message;
			break;
		case TIME_REQUEST:
			g_test_procs[0].m_priority = HIGH;
      g_test_procs[0].mpf_start_pc = &times_request_memory_block;
			break;

    case NOTHING:
      break;

    default:
      break;
  }
}


void alphabet(void)
{
  int i = 0;
  while (1) {
    for (i = 0; i < 26; i++) {
      uart1_put_char('A' + i);
    }

    uart1_put_string("\r\n");
    delay();
    release_processor();
  }
}

void numbers(void)
{
  int i = 0;
  while (1) {
    uart1_put_char('0' + i);
    uart1_put_string("\r\n");
    i++;
    i %= 10;
    delay();
    release_processor();
  }
}

void fibonacci(void)
{
  int a = 0;
  int b = 1;
  while (1) {
    int temp;
    uart1_put_char('0' + a);
    uart1_put_string("\r\n");
    temp = a;
    a = b;
    b = temp + b;
    delay();
    release_processor();
  }
}

void mem_proc(void)
{
  U32* my_mem[30] = {NULL};
  while (1) {
    int i;
    for ( i = 0; i < 30; i++ ) {
      my_mem[i] = request_memory_block();
    }
    for ( i = 0; i < 30; i++ ) {
      release_memory_block(my_mem[i]);
      my_mem[i] = NULL;
    }
    delay();
    release_processor();
  }
}

void priority_set(void)
{
  while (1) {
    set_process_priority(1, HIGH);
    delay();
  }
}

void memory_hog(void)
{
  while (1) {
    uart1_put_string("We are requesting and leaking memory.\r\n");
    request_memory_block(); // ask for infinite memory and leak it
  }
}

void memory_user_priority_setter(void)
{
  int i;
  U32* my_mem[30] = {NULL};

  for (i = 0; i < 30; i++) {
    my_mem[i] = request_memory_block();
  }

  set_process_priority(1, LOW);
  for (i = 0; i < 30; i++) {
    release_memory_block(my_mem[i]);
    my_mem[i] = NULL;
  }
  set_process_priority(1, HIGH);

  uart1_put_string("Some memory was taken used and freed up.\r\n");

  while (1) {
    release_processor();
  }
}

void nothing(void)
{
  while (1) {
    release_processor();
  }
}

void delay(void)
{
  int x = 0;
  for ( x = 0; x < 500000; x++); // some artifical delay
}

void sends_messages(void)
{
  int i;
  for (i = 1; i <= 6; i++) {
    char* s = "Sending to PID: ";
    int j = 0;
    MSGBUF* p_envelope = request_memory_block();
    p_envelope->mtype = DEFAULT;
    while (*s) {
      p_envelope->mtext[j++] = *(s++);
    }

    p_envelope->mtext[j++] = '0' + i;
    p_envelope->mtext[j++] = '\r';
    p_envelope->mtext[j++] = '\n';
    p_envelope->mtext[j++] = 0;

    uart1_put_string(p_envelope->mtext);

    p_envelope->mtext[0] = 'a' + i * 4;
    p_envelope->mtext[1] = '\r';
    p_envelope->mtext[2] = '\n';
    p_envelope->mtext[3] = 0;

    send_message(i, p_envelope);
  }

  set_process_priority(get_pid(), LOWEST);



  while (1) {
    release_processor();
  }
}

void receives_messages(void)
{
  int sender_id;
  MSGBUF* p_envelope = receive_message(&sender_id);
  uart1_put_string("Received message from PID ");
  uart1_put_char('0' + sender_id);
  uart1_put_string("\r\n");
  uart1_put_string("Message contents: ");
  uart1_put_string(p_envelope->mtext);

  release_memory_block(p_envelope);

  set_process_priority(get_pid(), LOWEST);

  while (1) {
    release_processor();
  }
}

void sends_CRT_messages(void)
{
  int i;
  for (i = 1; i <= 7; i++) {
    char* s = "SENDING MESSAGE TO CRT: ";
    int j = 0;
    MSGBUF* p_envelope = request_memory_block();
    p_envelope->mtype = DEFAULT;
    while (*s) {
      p_envelope->mtext[j++] = *(s++);
    }

    p_envelope->mtext[j++] = '0' + i;
    p_envelope->mtext[j++] = '\r';
    p_envelope->mtext[j++] = '\n';
    p_envelope->mtext[j++] = 0;

    send_message(PID_CRT, p_envelope);
  }

  set_process_priority(get_pid(), LOWEST);

  while (1) {
    release_processor();
  }
}

void sends_delayed_messages(void)
{
  MSGBUF* p_envelope_half = request_memory_block();
  MSGBUF* p_envelope_full = request_memory_block();
  char* s_half = "Half second delayed message.\r\n";
  char* s_full = "Full second delayed message.\r\n";

  p_envelope_half->mtype = DEFAULT;
  p_envelope_half->mtext[0] = 5;
  p_envelope_full->mtype = DEFAULT;
  p_envelope_full->mtext[0] = 10;

  delayed_send(get_pid(), p_envelope_half, 500);
  delayed_send(get_pid(), p_envelope_full, 1000);

  while (1) {
    MSGBUF* p_envelope = receive_message(NULL);
    if (p_envelope->mtext[0] == 5) {
      MSGBUF* p_print_msg = request_memory_block();
      char* s = s_half;
      int i = 0;

      p_print_msg->mtype = DEFAULT;
      while (*s) {
        p_print_msg->mtext[i++] = *s++;
      }
      p_print_msg->mtext[i++] = '\0';

      send_message(PID_CRT, p_print_msg);
      delayed_send(get_pid(), p_envelope, 500);
    } else {
      MSGBUF* p_print_msg = request_memory_block();
      char* s = s_full;
      int i = 0;

      p_print_msg->mtype = DEFAULT;
       p_print_msg->mtype = DEFAULT;
      while (*s) {
        p_print_msg->mtext[i++] = *s++;
      }
      p_print_msg->mtext[i++] = '\0';

      send_message(PID_CRT, p_print_msg);
      delayed_send(get_pid(), p_envelope, 1000);
    }
  }
}

void print_then_release_proc_1(void)
{
  uart1_put_string("ABCDEFGHIJKLMNOPQRSTUVWXYZ\r\n");
  while (1) {
    release_processor();
  }
}

void print_then_release_proc_2(void)
{
  uart1_put_string("1234567890\r\n");
  while (1) {
    release_processor();
  }
}

void set_priority_proc_1(void)
{
	while(1) {
		uart1_put_string("Proc 1\r\n");
	}
}

void set_priority_proc_2(void)
{
	while (1) {
			uart1_put_string("Proc 2\r\n");
	}
}

void times_send_message(void) 
{
	// returns ~ 3 as result = 3 microseconds
	void* msg = request_memory_block();
	int time2;
	int opTime;
	int time1 = ((LPC_TIM_TypeDef *) LPC_TIM1)->TC;
	send_message(1, msg);
	time2 = ((LPC_TIM_TypeDef *) LPC_TIM1)->TC;
	opTime = time2-time1;
	uart1_put_char('0' + opTime);
	
	while(1) {
		release_processor();
	}
}

void times_recieve_message(void) 
{
	// returns 3 as a result = 3 microseconds
	void* msg;
	int time1;
	int time2;
	int opTime;
	msg = request_memory_block();
	send_message(1, msg);
	time1 = ((LPC_TIM_TypeDef *) LPC_TIM1)->TC;
	msg = receive_message(NULL);
	time2 = ((LPC_TIM_TypeDef *) LPC_TIM1)->TC;
	opTime = time2-time1;
	uart1_put_char('0' + opTime);
	
	while(1) {
		release_processor();
	}
}

void times_request_memory_block(void) 
{
	// returns 4 as a result = 4 microseconds
	int time2;
	int opTime;
	int time1 = ((LPC_TIM_TypeDef *) LPC_TIM1)->TC;
	void* msg = request_memory_block();
	time2 = ((LPC_TIM_TypeDef *) LPC_TIM1)->TC;
	opTime = time2-time1;
	uart1_put_char('0' + opTime);
	
	while(1) {
		release_processor();
	}
}
	


