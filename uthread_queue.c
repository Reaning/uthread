/*
 *   FILE: uthread_queue.c
 * AUTHOR: Peter Demoreuille
 *  DESCR: queues of threads.
 *   DATE: Sun Sep  9 15:05:37 2001
 *
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "uthread.h"
#include "uthread_queue.h"


/*
 * utqueue_init
 * 初始化队列
 */
void utqueue_init(utqueue_t *q) {
    assert(q != NULL);

    list_init(&q->tq_waiters);
    q->tq_size = 0;
}

/*
 * utqueue_empty
 * 队列是否为空
 */
int utqueue_empty(utqueue_t *q) {
    assert(q != NULL);
    assert(list_empty(&q->tq_waiters) == (q->tq_size == 0));

    return (q->tq_size == 0);
}

/*
 * utqueue_enqueue
 * 将一个线程加入到队列
 */
void utqueue_enqueue(utqueue_t *q, uthread_t *thr) {
    assert(thr->ut_link.l_next == NULL && thr->ut_link.l_prev == NULL);

    list_insert_head(&q->tq_waiters, &thr->ut_link);
    q->tq_size++;
}

/*
 * utqueue_dequeue
 * 从队列中取出一个线程
 */
uthread_t * utqueue_dequeue(utqueue_t *q) {
    uthread_t *thr;
    list_link_t *link;

    assert(q != NULL);

    if (utqueue_empty(q))     {
        return NULL;
    }

    link = q->tq_waiters.l_prev;
    thr = list_item(link, uthread_t, ut_link);
    list_remove(link);

    q->tq_size--;

    return thr;
}

/*
 * utqueue_remove
 * 将指定的线程从队列中删除
 */
void utqueue_remove(utqueue_t *q, uthread_t *thr) {
    assert(thr->ut_link.l_next != NULL && thr->ut_link.l_prev != NULL);

    int found = 0;
    uthread_t *iterator;
    list_iterate_begin(&q->tq_waiters, iterator, uthread_t, ut_link) {
        if (iterator == thr) {
            found = 1;
            break;
        }
    } list_iterate_end();

    assert(found);

    list_remove(&thr->ut_link);
    q->tq_size--;
}
