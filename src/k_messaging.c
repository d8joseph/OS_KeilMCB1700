#include "k_messaging.h"
#include "k_rtx.h"
#include "k_process.h"

extern PCB** gp_pcbs;
extern PCB* gp_current_process;

int valid_recipient(int pid) {
  if (pid < 0 || pid > NUM_TOTAL_PROCS) {
    return 0;
  }

  switch (pid) {
    case PID_I_TIMER:
    case PID_I_UART:
    case PID_NULL:
    case PID_WALL_CLOCK:
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case PID_KCD:
    case PID_CRT:
    case PID_SET_PRIORITY:
		case PID_A:
		case PID_B:
		case PID_C:
      return 1;
    default:
      return 0;
  }
}

int k_send_message(int process_id, void* message_envelope)
{
  return k_send_message_proxied(gp_current_process->m_pid, process_id, message_envelope);
}

int k_send_message_proxied(int sender_pid, int receiver_pid, void* message_envelope)
{
  MEMORY_BLOCK* p_msg_block;
  PCB* p_pcb;

  if (sender_pid < 0 || sender_pid >= 16) {
    return RTX_ERR;
  }

  if (!valid_recipient(receiver_pid)) {
    return RTX_ERR;
  }

  p_pcb = gp_pcbs[receiver_pid];

  p_msg_block = (MEMORY_BLOCK*) ((U8*) message_envelope - sizeof(MEMORY_BLOCK_HEADER));
  p_msg_block->m_header.mp_owner = p_pcb;
  p_msg_block->m_header.mp_message_sender = gp_pcbs[sender_pid];
  p_msg_block->m_header.mp_message_recipient = p_pcb;
  p_msg_block->m_header.mp_next_block = NULL;

  if (p_pcb->mp_last_queued_message == NULL) {
    if (p_pcb->m_state == WAITING_ON_MSG) {
      *(p_pcb->mp_future_message) = p_msg_block;
      return queue_proc(p_pcb);
    } else {
      p_pcb->mp_first_queued_message = p_msg_block;
      p_pcb->mp_last_queued_message = p_msg_block;
    }
  } else {
    p_pcb->mp_last_queued_message->m_header.mp_next_block = p_msg_block;
    // no need to check if it is waiting on a message here
    // since the process's queue wasn't empty before
  }

  return RTX_OK;
}

void* k_receive_message(int* sender_id)
{
  if (gp_current_process->mp_first_queued_message != NULL) {
    MEMORY_BLOCK* p_msg_block;
    void* p_envelope;

    p_msg_block = gp_current_process->mp_first_queued_message;
    p_envelope = p_msg_block->mp_shared_mem;

    p_msg_block->m_header.mp_next_block = NULL;
    gp_current_process->mp_first_queued_message = gp_current_process->mp_first_queued_message->m_header.mp_next_block;
    if (gp_current_process->mp_first_queued_message == NULL) {
      gp_current_process->mp_last_queued_message = NULL;
    }

    if (sender_id != NULL) {
      *sender_id = p_msg_block->m_header.mp_message_sender->m_pid;
    }

    return p_envelope;
  } else {
    MEMORY_BLOCK* p_future_mem;
    gp_current_process->mp_future_message = &p_future_mem;
    gp_current_process->m_state = WAITING_ON_MSG;
    k_release_processor();

    if (sender_id != NULL) {
      *sender_id = p_future_mem->m_header.mp_message_sender->m_pid;
    }

    gp_current_process->mp_future_message = NULL;
    return (void*) p_future_mem->mp_shared_mem;
  }
}

int k_delayed_send(int process_id, void* message_envelope, int delay)
{
  MEMORY_BLOCK* p_msg_block;
  PCB* p_pcb;
  PCB* p_i_timer = gp_pcbs[PID_I_TIMER];

  if (!valid_recipient(process_id)) {
    return RTX_ERR;
  }

  if (delay < 0) {
    return RTX_ERR;
  }

  if (delay == 0) {
    return k_send_message(process_id, message_envelope);
  }

  p_pcb = gp_pcbs[process_id];

  p_msg_block = (MEMORY_BLOCK*) ((U8*) message_envelope - sizeof(MEMORY_BLOCK_HEADER));
  p_msg_block->m_header.mp_owner = gp_current_process;
  p_msg_block->m_header.mp_message_sender = gp_pcbs[PID_KCD]; // this signifies a delayed message
  p_msg_block->m_header.mp_message_recipient = p_pcb;
  p_msg_block->m_header.m_count = delay;
  p_msg_block->m_header.mp_next_block = NULL;

  if (p_i_timer->mp_last_queued_message == NULL) {
    if (p_i_timer->m_state == WAITING_ON_MSG) {
      *(p_i_timer->mp_future_message) = p_msg_block;
      return queue_proc(p_i_timer);
    } else {
      p_i_timer->mp_first_queued_message = p_msg_block;
      p_i_timer->mp_last_queued_message = p_msg_block;
    }
  } else {
    p_i_timer->mp_last_queued_message->m_header.mp_next_block = p_msg_block;
    // no need to check if it is waiting on a message here
    // since the process's queue wasn't empty before
  }

  return RTX_OK;
}
