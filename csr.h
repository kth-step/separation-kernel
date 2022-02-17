// See LICENSE file for copyright and license details.

#pragma once

#define read_csr(reg) ({ \
                register unsigned long _val; \
                __asm__("csrr %0,"#reg:"=r"(_val)); \
                _val; \
        })

