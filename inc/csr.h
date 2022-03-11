// See LICENSE file for copyright and license details.
#pragma once

#define read_csr(reg) ({ \
                register unsigned long _out; \
                __asm__("csrr %0,"#reg:"=r"(_out)); \
                _out; \
        })

#define write_csr(reg, _in) \
                __asm__("csrw "#reg",%0"::"r"(_in))

#define swap_csr(reg, _in) ({ \
                register unsigned long _out; \
                __asm__("csrr %0,"#reg",%1":"=r"(_out):"r"(_in)); \
                _out; \
        })
