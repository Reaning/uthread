/*
 *   FILE: uthread_sched.h
 * AUTHOR: Marcelo Martins
 *  DESCR: userland thread
 *   DATE: Mon Nov   10:00:08 2011
 * Modified for SCUT students in July 2021 By Wu Yimin
 */

#ifndef __uthread_sched_h__
#define __uthread_sched_h__

#include <sys/types.h>
#include "uthread.h"

int uthread_setprio(uthread_id_t id, int prio);
void uthread_yield(void);
void uthread_wake(uthread_t *uthr);
void uthread_nopreempt_on(void);
void uthread_nopreempt_off(void);
void uthread_switch(void);

#endif /* __uthread_sched_h__ */
