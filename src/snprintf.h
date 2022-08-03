#pragma once
#include <stdarg.h>
#include <stddef.h>

int snprintf(char* str, size_t n, const char* format, ...);

int vsnprintf(char* buf, size_t n, const char* format, va_list args);
