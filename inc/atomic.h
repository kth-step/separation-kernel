// See LICENSE file for copyright and license details.
#pragma once
#include <stdbool.h>
#include <stdint.h>

#define synchronize __sync_synchronize
#define fetch_and_and __sync_fetch_and_and
#define fetch_and_or __sync_fetch_and_or
#define fetch_and_add __sync_fetch_and_add
#define compare_and_swap __sync_bool_compare_and_swap
