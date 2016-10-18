/**
 * @file:   wall_clock.c
 * @brief:  establish the wall clock process
 * @author: pwrobel
 * @date:   2016/03/06
 */

#include "i_timer_proc.h"
#include "rtx.h"
#include "k_rtx.h"

// not declared in rtx.h since it should not be available to user procs
extern int k_send_message_proxied(int sender_pid, int receiver_pid, void *p_msg);
#define send_message_proxied(s_pid, r_pid, p_msg) _send_message_proxied((U32)k_send_message_proxied, s_pid, r_pid, p_msg)
extern int _send_message_proxied(U32 p_func, int s_pid, int r_pid, void *p_msg) __SVC_0;

PROC_INIT get_i_timer_proc_init(void) {
  PROC_INIT timer_proc_init;

  timer_proc_init.m_pid = PID_I_TIMER;
  timer_proc_init.m_priority = HIGH;
  timer_proc_init.m_stack_size = 0x600;
  timer_proc_init.mpf_start_pc = &i_timer_proc;

  return timer_proc_init;
}

void i_timer_proc(void) {
  MEMORY_BLOCK* p_delayed_messages = NULL;
  while(1) {
    int sender_id;
    MSGBUF* p_envelope;

    p_envelope = receive_message(&sender_id);
    switch (sender_id) {
      case PID_I_TIMER: {
        MEMORY_BLOCK* p_block = p_delayed_messages;
        MEMORY_BLOCK* p_last_block = NULL;

        while (p_block != NULL) {
          p_block->m_header.m_count--;
          if (p_block->m_header.m_count <= 0) {
            MEMORY_BLOCK* p_temp = p_block;
            if (p_last_block == NULL) {
              p_block = p_block->m_header.mp_next_block;
              p_delayed_messages = p_block;
            } else {
              p_block = p_block->m_header.mp_next_block;
              p_last_block->m_header.mp_next_block = p_block;
            }

            send_message_proxied(p_temp->m_header.mp_owner->m_pid, p_temp->m_header.mp_message_recipient->m_pid, p_temp->mp_shared_mem);
          } else {
            p_last_block = p_block;
            p_block = p_block->m_header.mp_next_block;
          }
        }

        if (p_envelope->mtype == TIMER_SIGNAL_STATIC_MEM) {
          p_envelope->mtext[0] = 0;
        } else {
          release_memory_block(p_envelope);
        }

        break;
      }

      case PID_KCD: { // represents a new delayed message to keep track of to send later
        MEMORY_BLOCK* p_block = (MEMORY_BLOCK*) ((U8*) p_envelope - sizeof(MEMORY_BLOCK_HEADER));
        p_block->m_header.mp_next_block = p_delayed_messages;
        p_delayed_messages = p_block;
        break;
      }

      default: {
        release_memory_block(p_envelope);
        break;
      }
    }
  }
}
