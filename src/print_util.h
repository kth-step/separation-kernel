// See LICENSE file for copyright and license details.
#pragma once

#include <stdint.h>
#include <stdio.h>

#include "config.h"

void print_relevant_config();

void print_relevant_config_locked();

void print_all_config();

void print_all_config_locked();

void print_sched(uint64_t schedule[]);


/* The following print functions utilize locks by default */
void p_string(const char * s);

void p_line(const char * s);

void p_line_i(const char * s, volatile int i);

void p_line_ul(const char * s, volatile uint64_t ul);

/* To allow more freeform printing, the lock can be used to wrap normal prints statments as well. 
   NOTE: these should not be used with above functions as they are already used where appropriate! */

void acquire_print_lock();

void release_print_lock();
