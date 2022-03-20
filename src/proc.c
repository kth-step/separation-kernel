// See LICENSE file for copyright and license details.
#include "config.h"
#include "proc.h"

Process processes[N_PROC];
Process current __asm__("tp");
