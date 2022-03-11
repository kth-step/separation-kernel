// See LICENSE file for copyright and license details.
#include "inc/config.h"
#include "inc/proc.h"

Process processes[N_PROC];
Process current __asm__("tp");
