/*
 *   FILE: uthread_private.h
 * AUTHOR: Peter Demoreuille
 *  DESCR: uthreads private stuff.
 *   DATE: Mon Oct  1 10:56:31 2001
 *
 */

#ifndef __uthread_private_h__
#define __uthread_private_h__

#include "uthread_mtx.h"

/*
 * 初始化调度器. 由 uthread_init() 调用
 */
void uthread_sched_init(void);


/*
 * 线程切换
 */
void uthread_switch();


/*
 * "idle" the "cpu".
 * see comment above uthread_switch()
 */
void uthread_idle(void);

#endif /* __uthread_private_h__ */
