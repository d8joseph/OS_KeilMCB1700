/**
 * @file:   set_priority.c
 * @brief:  establish the set_priority process
 * @author: bsankara
 * @date:   2016/03/18
 */

#include "set_priority.h"
#include "rtx.h"
#include <LPC17xx.h>
#include "uart.h"


PROC_INIT get_set_priority_init(void) {
  PROC_INIT set_priority_init;

  set_priority_init.m_pid = PID_SET_PRIORITY;
  set_priority_init.m_priority = HIGH;
  set_priority_init.m_stack_size = 0x200;
  set_priority_init.mpf_start_pc = &set_priority;

  return set_priority_init;
}

void set_priority(void) {
  int sender_id;
  int process_id = 0;
  int new_priority = 0;
  int retVal;
  char* err_msg = "Error changing priority.\r\n";
  // register self to kcd
  struct msgbuf *p_msg_env = (MSGBUF*) request_memory_block();
  p_msg_env->mtype = KCD_REG;
  p_msg_env->mtext[0] = '%';
  p_msg_env->mtext[1] = 'C';
  p_msg_env->mtext[2] = '\0';
  send_message(PID_KCD, (void *)p_msg_env);

  while (1) {
    char *m;
    MSGBUF* p_envelope = receive_message(&sender_id);

    process_id = 0;
    m = &p_envelope->mtext[3];

    while (*m) {
      if (*m == '\t' || *m == ' ') {
        m++;
      } else {
        break;
      }
    }

    while (*m) {
      if (*m >= '0' && *m <= '9') {
        process_id *= 10;
        process_id += *m - '0';
        m++;
        if (process_id > 100) { // to prevent overflowing into a valid id
          process_id = -1;
          break;
        }
      } else if (*m == '\t' || *m == ' ') {
        m++;
        break;
      } else {
        process_id = -1;
        break;
      }
    }

    if (!(*m)) {  // command should not be already done
      process_id = -1;
    }

    while (*m) {
      if (*m == '\t' || *m == ' ') {
        m++;
      } else {
        break;
      }
    }

		new_priority = 0;
    while (*m) {
      if (*m >= '0' && *m <= '9') {
        new_priority *= 10;
        new_priority += *m - '0';
        m++;
        if (new_priority > 100) { // to prevent overflowing into a valid priority
          new_priority = -1;
          break;
        }
      } else if (*m == '\t' || *m == ' ') {
        new_priority = -1;
        break;
      } else {
        new_priority = -1;
        break;
      }
    }

    while (*m) {
      if (*m == '\t' || *m == ' ') {
        m++;
      } else {
        break;
      }
    }

    if (*m) { // command should be done
      new_priority = -1;
    }

    if (p_envelope->mtext[2] != ' ' && p_envelope->mtext[2] != '\t') {  // command ID should be spaced
      new_priority = -1;
    }

    retVal = set_process_priority(process_id, new_priority);
    if (retVal == RTX_ERR) {
      char* s = err_msg;
      int i = 0;

      p_msg_env = (MSGBUF*) request_memory_block();
      p_msg_env->mtype = DEFAULT;
      while (*s) {
        p_msg_env->mtext[i++] = *s++;
      }
      p_msg_env->mtext[i++] = '\0';

      send_message(PID_CRT, p_msg_env);
    }

    release_memory_block(p_envelope);
  }
}
