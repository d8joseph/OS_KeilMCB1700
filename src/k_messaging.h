#ifndef K_MESSAGING_H
#define K_MESSAGING_H

int k_send_message(int process_id, void* message_envelope);
int k_send_message_proxied(int sender_pid, int receiver_pid, void* message_envelope);
void* k_receive_message(int* sender_id);
int k_delayed_send(int process_id, void* message_envelope, int delay);

#endif
