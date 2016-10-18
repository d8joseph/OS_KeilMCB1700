/**
 * @file:   wall_clock.h
 * @brief:  establish the wall clock process
 * @author: bsankara
 * @date:   2016/03/06
 */

#ifndef WALL_CLOCK_H_
#define WALL_CLOCK_H_

#include "rtx.h"

PROC_INIT get_wall_clock_init(void);
void wall_clock(void);

#endif
