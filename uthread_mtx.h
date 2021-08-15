/*
 *   FILE: uthread_mtx.h
 * AUTHOR: Peter Demoreuille
 *  DESCR: userland mutexes
 *   DATE: Sat Sep  8 12:37:18 2001
 *
 */

#ifndef __uthread_mtx_h__
#define __uthread_mtx_h__

#include "uthread.h"
#include "uthread_queue.h"


typedef struct uthread_mtx {
    struct uthread  *m_owner;
    utqueue_t       m_waiters;
} uthread_mtx_t;

void uthread_mtx_init(uthread_mtx_t *mtx);
void uthread_mtx_lock(uthread_mtx_t *mtx);
void uthread_mtx_unlock(uthread_mtx_t *mtx);
int uthread_mtx_trylock(uthread_mtx_t *mtx);


#endif /* __uthread_mtx_h__ */
