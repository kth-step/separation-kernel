#include "kprint.h"

#include "lock.h"
#include "platform.h"
#include "snprintf.h"

void kprint(const char* s)
{
        static lock_t lock = INIT_LOCK;
        lock_acquire(&lock);
        while (*s != '\0')
                uart_putchar(*s++);
        lock_release(&lock);
}

void kprintf(const char* format, ...)
{
        va_list args;
        va_start(args, format);
        char buf[256];
        s3k_snvprintf(buf, 256, format, args);
        kprint(buf);
}
