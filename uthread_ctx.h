/*
 *   FILE: uthread_ctx_linux.h
 * AUTHOR: Peter Demoreuille
 *  DESCR: userland thread context switching for solaris
 *   DATE: Sat Sep  8 11:11:43 2001
 *
 */

#ifndef __uthread_linux_ctx_h__
#define __uthread_linux_ctx_h__

#ifdef _REENTRANT
#error Compiling -mt is NOT supported
#endif

#include <ucontext.h>

typedef ucontext_t  uthread_ctx_t;



/* Sets up the given context with a stack and a function to execute with
 * provided arguments. A call to uthread_(set/swap)context will cause
 * func to begin executing.
 */
void uthread_makecontext(uthread_ctx_t *ctx, char *stack, int stacksz,
                         void (*func)(), long arg1, void *arg2);

/* Sets the current context to ctx. This method will never return.
*/
void uthread_setcontext(uthread_ctx_t *ctx);

/* Get the context which is currently executing.
*/
/* void uthread_getcontext(uthread_ctx_t *ctx); */
#define uthread_getcontext(ctx) \
    do { \
        assert(ctx);  \
        getcontext(ctx); \
    } while(0);

/* Causes oldctx to yield the processor to newctx. A subsequent call to
 * uthread_swapcontext with oldctx as the target will cause the old
 * context to continue executing as if uthread_swapcontext returned from
 * the initial call without doing anything.
 */
void uthread_swapcontext(uthread_ctx_t *oldctx, uthread_ctx_t *newctx);

#endif /* __uthread_ctx_linux_h__ */
