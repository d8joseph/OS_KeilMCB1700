/**
 * @file:   stress_test.h
 * @brief:  establish the stress test process
 * @author: bsankara
 * @date:   2016/03/20
 */

#ifndef STRESS_TEST_H_
#define STRESS_TEST_H_

#include "rtx.h"

PROC_INIT get_process_a_init(void);
PROC_INIT get_process_b_init(void);
PROC_INIT get_process_c_init(void);
void stress_process_a(void);
void stress_process_b(void);
void stress_process_c(void);

#endif
