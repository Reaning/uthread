/*
 *   FILE: uthread.c
 * AUTHOR: peter demoreuille
 *  DESCR: userland threads
 *   DATE: Sun Sep 30 23:45:00 EDT 2001
 *
 *
 * Modified to handle time slicing by Tom Doeppner
 *   DATE: Sun Jan 10, 2016
 * Further modifications in January 2020
 * Modified for SCUT students in July 2021 By Wu Yimin
 */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/errno.h>
#include <sys/types.h>
#include <unistd.h>

#include "uthread.h"
#include "uthread_private.h"
#include "uthread_queue.h"
#include "uthread_bool.h"
#include "uthread_sched.h"
#include "uthread_mtx.h"
#include "uthread_cond.h"


/* ---------- globals -- */

uthread_t    *ut_curthr = NULL;             /* 当前正在执行的线程 */
uthread_t    uthreads[UTH_MAX_UTHREADS];    /* 系统中的所有线程 */

static list_t           reap_queue;         /* 已经运行结束，但资源尚未回收（即死线程）队列 */
static uthread_id_t     _thr_id;      /* 清理线程，用于回收死线程资源的线程 */


/* ---------- prototypes -- */

static void create_first_thr(void);

static uthread_id_t uthread_alloc(void);
static void uthread_destroy(uthread_t *thread);

static char *alloc_stack(void);
static void free_stack(char *stack);

static void _init1(void);
static void _init2(void);
static void (long a0, char *a1[]);
static void make_reapable(uthread_t *uth);



/* ---------- public code -- */

/*
 * uthread_init，这个函数只在用户进程启动时调用一次。
 *
 * 初始化所有的全局数据结构和变量. 
 *
 * 这个函数需要设置每一个线程的 ut_state 和 ut_id , 简单起见，本系统中选择线程数组的下标作为 ut_id .
 */
void uthread_init(void) {
    for(uthread_id_t i = 0;i < UTH_MAX_UTHREADS;i++){
        uthreads[i].ut_state = UT_NO_STATE;
        uthreads[i].ut_id = i;
    }
    /* 以下函数代码不需要修改 */

    uthread_sched_init();  //不需要修改，初始化优先级的队列
    _init1();        //创建互斥锁和条件变量 
    create_first_thr();    //不需要修改，创建第一个线程，即主线程
    _init2();        //不需要修改，创建reap线程
}



/*
* 创建一个线程执行指定的函数 <func>，函数的参数为 <arg1> 和 <arg2>，优先级为 <prio>.
*
* 首先，使用 uthread_alloc 找到一个有效的 id，找不到时，返回合适的错误.
*
* 然后, 为线程分配栈, 分配不成功时返回合适的错误.
*
* 使用uthread_makecontext() 创建线程的上下文.
*
* 按照新发现的线程 id，设置 uthread_t 结构, 调用 uthread_setprio 设置线程的优先级，
* 并设置线程的状态为 UT_RUNNABLE， 在 <uidp> 中返回线程 id. 
*
* 成功时返回 0 .
*/

int uthread_create(uthread_id_t *uidp, uthread_func_t func,
               long arg1, char *arg2[], int prio) {
    uthread_id_t id= uthread_alloc();
    if ( id == -1 )return -1;
    else{   
        *uidp = id;
        uthread_t *thr = &uthreads[id];
        thr->ut_stack = alloc_stack();
        if(thr->ut_stack == NULL)return -1;
        if ( thr->ut_stack ){
            uthread_makecontext(&thr->ut_ctx,thr->ut_stack, UTH_STACK_SIZE, func, arg1, arg2);
            memset(&thr->ut_link, 0, sizeof(list_link_t));
            thr->ut_id = id;
            thr->ut_state = UT_RUNNABLE;
            thr->ut_errno = 0;
            thr->ut_has_exited = 0;
            thr->ut_exit = NULL;
            thr->ut_detach_state = UT_DETACHABLE;
            thr->ut_waiter = NULL;
            thr->ut_prio = -1;
            uthread_setprio(thr->ut_id, prio);
            return 0;
        }
    }
    return -1;
}

/*
 * uthread_exit
 *
 * 结束当前的线程.  注意设置 uthread_t 中的标志.
 *
 * 如果不是一个 detached thread, 并且有一个线程等待 to join with it, 则唤醒那个线程 thread.
 *
 * 如果线程是 UT_DETACHABLE, 则通过调用 make_reapable()将其放入清理线程（） 清理队列
 * 并唤醒清理清理线程.
 * 如果线程是 UT_JOINABLE，则唤醒等待的线程。
 * 然后调用 uthread_switch() 切换线程。
 */
void uthread_exit(void *status) {
    ut_curthr->ut_has_exited = 1;
    ut_curthr->ut_exit = status;
    if(ut_curthr->ut_waiter != NULL){
        if(ut_curthr->ut_state == UT_DETACHABLE){
            make_reapable(ut_curthr);
        }else{
            if(ut_curthr->ut_waiter != NULL){
                uthread_wake(ut_curthr->ut_waiter);
            }
        }
    }
    ut_curthr->ut_state = UT_WAIT;
    uthread_switch();
    // PANIC("returned to a dead thread");
}

/*
 * uthread_join
 *
 * 等待指定的线程结束. 如果线程没有结束执行，调用线程需要阻塞，直到这一事件发生.
 *
 * 错误条件包括 (但不限于):
 * o 由 <uid> 描述的线程不存在
 * o 2个线程试图 join 同一个线程等
 * 返回合适的 error code (参考 pthread_join 的 manpage) .
 * 如果要等待的线程还没有结束，则置线程的等待线程为当前线程，线程状态改为 UT_WAIT，切换线程
 *
 * 如果成功地等到了线程结束， 则调用 make_reapable 唤醒清理线程  将其彻底清理 .
 */
int uthread_join(uthread_id_t uid, void **return_value) {
    if(uid >= UTH_MAX_UTHREADS){
        return -1;
    }
    uthread_t *thr = &uthreads[uid];
    if(thr->ut_detach_state==UT_DETACHABLE)
    {   
        if(return_value!=NULL)*return_value=NULL;
        return -1;
    }
    if(thr->ut_waiter != NULL || thr->ut_state == UT_NO_STATE){
        return -1;
    }
    if(thr->ut_state != UT_ZOMBIE){
        thr->ut_waiter = ut_curthr;
        ut_curthr->ut_state = UT_WAIT;
        uthread_switch();
    }
    *return_value = thr->ut_exit;
    make_reapable(thr);
    return 0;
}

/*
 * uthread_self
 *
 * 返回当前正在执行的线程的 id.
 */
uthread_id_t uthread_self(void) {
    assert(ut_curthr != NULL);
    return ut_curthr->ut_id;
}

/* ------------- private code -- */

/*
 * uthread_alloc
 *
 * 找到一个自由的 uthread_t, 返回其 id (uthread_id_t).
 */
static uthread_id_t uthread_alloc(void) {
    for(uthread_id_t i = 0;i < UTH_MAX_UTHREADS;i++){
        if(uthreads[i].ut_state == UT_NO_STATE)return i;
    }
    return -1;
}

/*
 * uthread_destroy
 *
 * 清理指定的线程
 */
static void uthread_destroy(uthread_t *uth) {
    assert(uth->ut_state == UT_ZOMBIE);

    free_stack(uth->ut_stack);
    // printf("123333\n");
    // uth->ut_has_exited = 0;
    // uth->ut_errno = uth->ut_has_exited;
    // uth->ut_detach_state = UT_DETACHABLE;
    // uth->ut_exit = NULL;
    // uth->ut_waiter = NULL;
    // uth->ut_state = UT_NO_STATE;

    uth->ut_state = UT_NO_STATE;
    uth->ut_has_exited = 1;
    uth->ut_waiter = NULL;
}


/****************************************************************************
 * 本文件中的以下代码不需要任何修改
 ****************************************************************************/

static uthread_mtx_t reap_mtx;
static uthread_cond_t reap_cond;  


static void _init1(void) {   //初始化线程相关
    list_init(&reap_queue);        //初始化队列，这意味着废弃的线程要入队
    uthread_mtx_init(&reap_mtx);   //初始化与相关的互斥锁
    uthread_cond_init(&reap_cond); //初始化与相关的条件变量
}

static void _init2(void) {  //启动线程
    uthread_create(&_thr_id, , 0, NULL, UTH_MAXPRIO);

    assert(_thr_id != -1);
}

#ifdef CLOCKCOUNT
extern int clock_count;
extern int taken_clock_count;
#endif

/*
 *  负责遍历死线程列表（reap_queue） 中的所有线程 (均应当处于 ZOMBIE 状态)，
 * 并清除这些线程.
 *
 * 此外，如果再没有其他线程 (除了  自己)，它将调用 exit() 结束进程.
 */
static void (long a0, char *a1[]) {
    uthread_mtx_lock(&reap_mtx);
    while(1)    {
        uthread_t   *thread;
        int         th;

        while(list_empty(&reap_queue)) { //循环等待reap_queue不空
            uthread_cond_wait(&reap_cond, &reap_mtx);
        }

        /* 迭代reap_queue，逐一取出死线程，将其移除reap_queue，然后销毁   */
        list_iterate_begin(&reap_queue, thread, uthread_t, ut_link) {
            list_remove(&thread->ut_link);
            uthread_destroy(thread);
        }
        list_iterate_end();    //迭代结束
        
        for (th = 0; th < UTH_MAX_UTHREADS; th++) { /* 检查所有线程，是否还有线程 */
            if (th != _thr_id &&
                uthreads[th].ut_state != UT_NO_STATE) {
                break;
            }
        }

        if (th == UTH_MAX_UTHREADS) { //没有其他线程了，打印一些信息，进程结束
            /* we leak the 's stack */
            fprintf(stderr, "uthreads: no more threads.\n");
            fprintf(stderr, "uthreads: bye!\n");
            exit(0);
        }
    }
}

/*
 * 将主线程设置成一个普通线程，使其可以被调度。
 * 这个函数只调用一次，由main 函数中的 uthread_init() 调用.
 */
static void create_first_thr(void) {
    uthread_id_t tid = 0; // 第一个线程的 ID 写死为 0 号线程
    ut_curthr = &uthreads[tid];
    memset(&ut_curthr->ut_link, 0, sizeof(list_link_t));
    uthread_getcontext(&ut_curthr->ut_ctx);
    ut_curthr->ut_prio = UTH_MAXPRIO;
    ut_curthr->ut_errno = ut_curthr->ut_has_exited = ut_curthr->ut_no_preempt_count = 0;
    ut_curthr->ut_detach_state = UT_DETACHABLE;
    ut_curthr->ut_exit = ut_curthr->ut_waiter = NULL;
    ut_curthr->ut_state = UT_ON_CPU;
}

/*
 * 将指定的线程放入到死线程队列, 并唤醒 .
 *
 */

static void make_reapable(uthread_t *uth) {
    assert(ut_curthr->ut_state != UT_ZOMBIE);
    uthread_mtx_lock(&reap_mtx);
    uth->ut_state = UT_ZOMBIE;
    list_insert_tail(&reap_queue, &uth->ut_link);
    uthread_cond_signal(&reap_cond);
    uthread_mtx_unlock(&reap_mtx);
}

static char * alloc_stack(void) {
    char *stack = (char *)malloc(UTH_STACK_SIZE);
    return stack;
}

static void free_stack(char *stack) {
    free(stack);
}
