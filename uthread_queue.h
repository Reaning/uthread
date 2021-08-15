/*
 *   FILE: uthread_queue.h
 * AUTHOR: Peter Demoreuille
 *  DESCR: a queue of threads
 *   DATE: Sun Sep  9 15:02:12 2001
 *
 */

#ifndef __uthread_queue_h__
#define __uthread_queue_h__

#include "list.h"
#include "uthread.h"


typedef struct utqueue {
    list_t  tq_waiters;
    int     tq_size;
} utqueue_t;

/* ------------------ prototypes -- */


void utqueue_init(utqueue_t *q);
int utqueue_empty(utqueue_t *q);
void utqueue_enqueue(utqueue_t *q, struct uthread *thr);
void utqueue_remove(utqueue_t *q, struct uthread *thr);
struct uthread *utqueue_dequeue(utqueue_t *q);


#endif /* __uthread_queue_h__ */
