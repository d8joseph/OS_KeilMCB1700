/**
 * @file:   wall_clock.h
 * @brief:  establish the wall clock process
 * @author: pwrobel
 * @date:   2016/03/06
 */

#ifndef TIMER_PROC_H_
#define TIMER_PROC_H_

#include "rtx.h"

PROC_INIT get_i_timer_proc_init(void);
void i_timer_proc(void);

#endif
