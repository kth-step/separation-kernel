// See LICENSE file for copyright and license details.
#pragma once

#include "config.h"

#define read_csr(reg)                                          \
        ({                                                     \
                register unsigned long out;                    \
                __asm__ volatile("csrr %0," #reg : "=r"(out)); \
                out;                                           \
        })

#define write_csr(reg, _in) __asm__ volatile("csrw " #reg ",%0" ::"r"(_in))

#define swap_csr(reg, in)                               \
        ({                                              \
                register unsigned long _out;            \
                __asm__ volatile("csrrw %0," #reg ",%1" \
                                 : "=r"(_out)           \
                                 : "r"(in));            \
                _out;                                   \
        })

#define set_csr(reg, in) ({ __asm__ volatile("csrs " #reg ",%0" :: "r"(in)); })

#define clear_csr(reg, in) \
        ({ __asm__ volatile("csrc " #reg ",%0" :: "r"(in)); })

#define get_software_hartid()                   \
        ({                                      \
                read_csr(mhartid) - MIN_HARTID; \
        })
