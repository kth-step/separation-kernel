#pragma once

static inline unsigned long long preemption_enable(void)
{
        unsigned long long out;
        asm volatile("csrrsi %0,mstatus,8"
                     : "=r"(out));
        return out;
}

static inline unsigned long long preemption_disable(void)
{
        unsigned long long out;
        asm volatile("csrrci %0,mstatus,8"
                     : "=r"(out));
        return out;
}

static inline void preemption_restore(unsigned long long prev)
{
        asm volatile("csrw mstatus,%0" ::"r"(prev));
}
