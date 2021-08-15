/*
 * FILE: uthread_ctx_solaris.c
 * AUTHOR: Peter Demoreuille
 * DESCR: linux/solaris userland thread ctx swill
 * DATE: Sat Sep 8 11:26:15 2001
 *
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "uthread.h"
#include "uthread_ctx.h"
#include "uthread_sched.h"


static void thread_start(void (*func)(), long arg1, void *arg2);

/* -------------------- public code -- */


void uthread_makecontext(uthread_ctx_t *ctx, char *stack, int stacksz,
                    void (*func)(), long arg1, void *arg2) {
    int rv;

    assert(ctx != NULL);
    assert(stack != NULL && stacksz > 0);

    /* initialize the context */
    rv = getcontext(ctx);
    assert(rv != -1);

    /* set the new stack */
    ctx->uc_stack.ss_sp = (void*)stack;
    ctx->uc_stack.ss_size = stacksz;
    ctx->uc_stack.ss_flags = 0;
    ctx->uc_link = NULL;

    /* makecontext */
    makecontext(ctx, (void(*)())thread_start, 3, func, arg1, arg2);

}


void uthread_setcontext(uthread_ctx_t *ctx) {
    assert(ctx != NULL);
    setcontext(ctx);
}

void uthread_swapcontext(uthread_ctx_t *oldctx, uthread_ctx_t *newctx) {
    assert(oldctx != NULL && newctx != NULL);
    swapcontext(oldctx, newctx);
}

/* ------------------- private code -- */

static void thread_start(void (*func)(), long arg1, void *arg2) {
    assert(func != NULL);
    (func)(arg1, arg2);
    uthread_exit(0);
}
