// See LICENSE file for copyright and license details.
#include "kprint.h"

#include "lock.h"
#include "platform.h"
#include "snprintf.h"

int kprintf(const char* format, ...)
{
        va_list args;
        char buf[128];
        va_start(args, format);
        vsnprintf(buf, 128, format, args);
        return puts(buf);
}

int puts(const char* s)
{
        static lock_t lock = INIT_LOCK;
        int i;
        lock_acquire(&lock);
        for (i = 0; s[i] != '\0'; i++) {
                uart_putchar(s[i]);
        }
        lock_release(&lock);
        return i;
}
