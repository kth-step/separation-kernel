// See LICENSE file for copyright and license details.
#pragma once
#include <stdbool.h>
#include <stdint.h>

#define synchronize() __sync_synchronize()
#define fetch_and_and(ptr, val) __sync_fetch_and_and(ptr, val)
#define fetch_and_or(ptr, val) __sync_fetch_and_or(ptr, val)
#define fetch_and_add(ptr, val) __sync_fetch_and_add(ptr, val)

#define compare_and_set(ptr, expected, desired) __sync_bool_compare_and_swap(ptr, expected, desired)
#define compare_and_swap(ptr, expected, desired) __sync_val_compare_and_swap(ptr, expected, desired)
