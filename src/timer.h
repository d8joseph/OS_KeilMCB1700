/**
 * @brief timer.h - Timer header file
 * @author Y. Huang
 * @date 2013/02/12
 */
#ifndef _TIMER_H_
#define _TIMER_H_

#include "k_rtx.h"

PROC_INIT get_i_timer_proc_init(void);
U32 timer_init(U8 n_timer);  /* initialize timer n_timer */

#endif /* ! _TIMER_H_ */
