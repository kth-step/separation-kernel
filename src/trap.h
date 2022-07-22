#pragma once

void trap_recv_yield(void) __attribute__((noreturn));

void sys_to_sched(void) __attribute__((noreturn));