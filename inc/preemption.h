// See LICENSE file for copyright and license details.
#pragma once

static inline unsigned long long preemption_enable(void)
{
        unsigned long long out;
        __asm__ volatile("csrrsi %0,mstatus,8" : "=r"(out));
        return out;
}

static inline unsigned long long preemption_disable(void)
{
        unsigned long long out;
        __asm__ volatile("csrrci %0,mstatus,8" : "=r"(out));
        return out;
}

static inline void preemption_restore(unsigned long long prev)
{
        __asm__ volatile("csrw mstatus,%0" ::"r"(prev));
}
