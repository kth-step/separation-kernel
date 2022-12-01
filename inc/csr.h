// See LICENSE file for copyright and license details.
#pragma once

#define read_csr(reg)                                          \
        ({                                                     \
                register uint64_t out;                         \
                __asm__ volatile("csrr %0," #reg : "=r"(out)); \
                out;                                           \
        })

#define write_csr(reg, _in) __asm__ volatile("csrw " #reg ",%0" ::"r"(_in))

#define swap_csr(reg, in)                                                       \
        ({                                                                      \
                register uint64_t out;                                          \
                __asm__ volatile("csrr %0," #reg ",%1" : "=r"(_out) : "r"(in)); \
                out;                                                            \
        })

#define set_csr(reg, in) __asm__ volatile("csrs " #reg ",%0" ::"r"(in))

#define clear_csr(reg, in) __asm__ volatile("csrc " #reg ",%0" ::"r"(in))
