#pragma once
#include <stdarg.h>
#include <stddef.h>

void s3k_snprintf(char* str, size_t n, const char* format, ...);

void s3k_snvprintf(char* buf, size_t n, const char* format, va_list args);
