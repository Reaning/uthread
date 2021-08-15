/*
 *   FILE: uthread_cond.h
 * AUTHOR: Peter Demoreuille
 *  DESCR: uthreads condition variables
 *   DATE: Mon Oct  1 00:16:54 2001
 *
 */

#ifndef __uthread_cond_h__
#define __uthread_cond_h__

#include "uthread_mtx.h"
#include "uthread_queue.h"


typedef struct uthread_cond {
    struct utqueue  uc_waiters;
} uthread_cond_t;


void uthread_cond_init(uthread_cond_t *);
void uthread_cond_wait(uthread_cond_t *, struct uthread_mtx *);
void uthread_cond_signal(uthread_cond_t *);
void uthread_cond_broadcast(uthread_cond_t *);



#endif /* __uthread_cond_h__ */
