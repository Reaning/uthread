/*
 *   FILE: uthread_sched.c
 * AUTHOR: Peter Demoreuille
 *  DESCR: scheduling wack for uthreads
 *   DATE: Mon Oct  1 00:19:51 2001
 *
 * Modified to handle time slicing by Tom Doeppner
 *   DATE: Sun Jan 10, 2016
 * Further modifications in January 2020
 * Modified for SCUT students in July 2021 By Wu Yimin
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/errno.h>
#include <signal.h>
#include <sys/time.h>

#include "uthread.h"
#include "uthread_private.h"
#include "uthread_ctx.h"
#include "uthread_queue.h"
#include "uthread_bool.h"
#include "uthread_sched.h"

/* ---------- globals -- */

static utqueue_t runq_table[UTH_MAXPRIO + 1]; /* 优先级队列，每个优先级一个队列 */

/* ----------- public code -- */

/*
 * uthread_yield
 *
 * 当前正在运行的线程让出CPU。线程仍然处于可运行状态，即 UT_RUNNABLE， 即：可被调度器调度。 
 * 这个函数返回时，线程再次执行. 也就是说:
 * 调用此函数时, 当前线程加入到就绪队列.
 * 如果当前线程大于系统中已有的最高优先级线程，则从直接从 uthread_yield 返回
 * 否则放弃 CPU，转去执行最高优先级的线程.
 */
void uthread_yield(void) {
        Function_you_need_to_implement("UTHREADS: uthread_yield");
    
}

/*
 * uthread_wake
 *
 * 唤醒指定的线程，使其可被在执行。线程有可能已经处于就绪态，注意处理这种情况。
 * 所作的事情：改变状态，将其放入就绪队列。
 */
void uthread_wake(uthread_t *uthr) {
        Function_you_need_to_implement("UTHREADS: uthread_wake");
    
}

/*
 * uthread_setprio
 *
 * 改变指定线程的优先级.  注意，如果线程已处于 UT_RUNNABLE 状态 (可执行，但没有占有 cpu)，
 * 则应该改变其优先级队列. 如果线程的优先级高于当前调用者，则调用者还要放弃CPU .
 * 如果线程状态为 UT_TRANSITION, 则它是一个全新的线程，即将第一次被放入就绪队列 . 记住将其状态设置为 UT_RUNNABLE 。
 * 成功时返回 1 , 否则返回 0 .
 */
int uthread_setprio(uthread_id_t id, int prio) {
        Function_you_need_to_implement("UTHREADS: uthread_setprio");
    return 0;
    
}

/* ----------- private code -- */

/*
 * uthread_switch()
 *
 * 所有的魔力在此.  找到最高优先级的可运行线程，然后使用 uthread_swapcontext() 切换到它.  
 * 不要忘记设置 UT_ON_CPU 线程状态和当前线程. 注：如果调用线程本身就是最高优先级线程，则切换回调用线程.
 * 主要工作：找出系统中优先级最高的线程，并切换
 * 必须要有可执行线程!
 */
void uthread_switch() {
        Function_you_need_to_implement("UTHREADS: uthread_switch");

    PANIC("no runnable threads");
    
}

void uthread_sched_init(void) {
    int i;
    for (i = 0; i <= UTH_MAXPRIO; i++)     {
        utqueue_init(&runq_table[i]);
    }
}

