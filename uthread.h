/*
 *   FILE: uthread.h
 * AUTHOR: Peter Demoreuille
 *  DESCR: userland threads
 *   DATE: Sat Sep  8 10:56:08 2001
 *
 * Modified for use with preemptive scheduling in January 2020 by Tom Doeppner
 * 
 * Modified for SCUT students in July 2021 By Wu Yimin
 *
 */

#ifndef __uthread_h__
#define __uthread_h__

#include <sys/types.h>
#include "uthread_ctx.h"


#include "list.h"


/* -------------- defs -- */

#define UTH_MAXPRIO             7               
/* 线程的最大优先级，优先级数字越大，优先级越高 */
#define UTH_MAX_UTHREADS        64              /* 系统支持的最多的线程数 */
#define UTH_STACK_SIZE          64*1024         /* 每个线程的栈的大小 */

#define Function_you_need_to_implement(msg) \
    do { \
        fprintf(stderr, "Need to implement at %s:%i -- %s\n", \
                __FILE__, __LINE__, (msg)); \
    } while(0);

#define PANIC(err) \
    do { \
        fprintf(stderr, "PANIC at %s:%i -- %s\n", \
                __FILE__, __LINE__, err); \
        abort(); \
    } while(0);

#undef errno
#define errno   (ut_curthr->ut_errno)

typedef int uthread_id_t;
typedef void(*uthread_func_t)(long, char *argv[]);

typedef enum  {
    UT_NO_STATE,        /* 无效的线程状态 */
    UT_ON_CPU,          /* 线程正在执行 */
    UT_RUNNABLE,        /* 线程可运行，就绪 */
    UT_WAIT,            /* 线程被阻塞 */
    UT_ZOMBIE,          /* 线程处于 zombie 状态，即已结束，但需要回收资源 */
    UT_TRANSITION,      /* 线程处于创建状态 */    
    UT_JOINABLE,           //线程结束时需要一个线程回收其资源
    UT_DETACHABLE,          //线程结束时，不需要一个线程回收其资源 
    UT_NUM_THREAD_STATES    //线程状态数目
} uthread_state_t;

/* --------------- thread structure -- */

typedef struct uthread {
    list_link_t         ut_link;                /* 用法1：连接所有的死线程link on waitqueue / scheduler */
    uthread_ctx_t       ut_ctx;                 /* 上下文 */
    char                *ut_stack;              /* 线程所使用的栈 */
    uthread_id_t        ut_id;                  /* 线程 id */
    uthread_state_t     ut_state;               /* 线程状态 */
    int                 ut_prio;                /* 线程的优先级，数字越大优先级越高 */
    int                 ut_errno;               /* 线程的错误码 */
    int                 ut_has_exited;          /* 线程是否已结束? */
    void                *ut_exit;               /* 线程的退出码 */
    int                 ut_detach_state;            /* 线程是 detachable 还是 joinable,默认：UT_DETACHABLE */
    int                 ut_no_preempt_count;    /* 用于禁止抢占，留待今后实现抢占式调度使用，非抢占式调度或协程不用 */
    struct uthread      *ut_waiter;             /* 等待该线程结束的线程*/
} uthread_t;

/* --------------- prototypes -- */

extern uthread_t uthreads[UTH_MAX_UTHREADS];     //所有的线程
extern uthread_t *ut_curthr;                     //当前正在执行的线程

void uthread_init(void);

int uthread_create(uthread_id_t *id, uthread_func_t func, long arg1,
                   char *arg2[], int prio);
void uthread_exit(void *status);
uthread_id_t uthread_self(void);

int uthread_join(uthread_id_t id, void **exit_value);
//int uthread_detach(uthread_id_t id);

#endif /* __uthread_h__ */
