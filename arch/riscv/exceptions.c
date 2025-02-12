/*
 * Copyright (c) 2015 Travis Geiselbrecht
 *
 * Use of this source code is governed by a MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT
 */
#include <assert.h>
#include <lk/compiler.h>
#include <lk/trace.h>
#include <arch/riscv.h>
#include <kernel/thread.h>

#define LOCAL_TRACE 0

// keep in sync with asm.S
struct riscv_short_iframe {
    ulong  mepc;
    ulong  mstatus;
    ulong  ra;
    ulong  a0;
    ulong  a1;
    ulong  a2;
    ulong  a3;
    ulong  a4;
    ulong  a5;
    ulong  a6;
    ulong  a7;
    ulong  t0;
    ulong  t1;
    ulong  t2;
    ulong  t3;
    ulong  t4;
    ulong  t5;
    ulong  t6;
};

extern enum handler_return riscv_platform_irq(void);

void riscv_exception_handler(ulong cause, ulong epc, struct riscv_short_iframe *frame) {
    LTRACEF("cause %#lx epc %#lx\n", cause, epc);

    DEBUG_ASSERT(arch_ints_disabled());
    DEBUG_ASSERT(frame->mstatus & RISCV_STATUS_MPIE);

    enum handler_return ret = INT_NO_RESCHEDULE;
    switch (cause) {
        case 0x80000007: // machine timer interrupt
            ret = riscv_timer_exception();
            break;
        case 0x8000000b: // machine external interrupt
            ret = riscv_platform_irq();
            break;
        default:
            TRACEF("unhandled cause %#lx, epc %#lx, mtval %#lx\n", cause, epc, riscv_csr_read(mtval));
            panic("stopping");
    }

    DEBUG_ASSERT(arch_ints_disabled());
    DEBUG_ASSERT(frame->mstatus & RISCV_STATUS_MPIE);

    if (ret == INT_RESCHEDULE) {
        thread_preempt();
    }

    DEBUG_ASSERT(arch_ints_disabled());
    DEBUG_ASSERT(frame->mstatus & RISCV_STATUS_MPIE);
}
