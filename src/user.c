// See LICENSE file for copyright and license details.
#include <stdint.h>

#include "config.h"
#include "kprint.h"
#include "lock.h"
#include "s3k.h"

#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))

#define MSG_SIZE 20
static volatile char uart_input[MSG_SIZE + 1];
static volatile char app_output[2][MSG_SIZE + 1];
static volatile char uart_output[2][MSG_SIZE + 1];
static volatile int cntr;

extern void user_code();

void dump_cap(char* name)
{
    static lock_t lock = INIT_LOCK;
    lock_acquire(&lock);
    cap_t cap;
    char buf[256];

    kprintf("\n%s capabilities\n", name);
    for (int i = 0; i < 32; i++) {
        if (s3k_read_cap(i, &cap) == S3K_OK && cap_get_type(cap) != CAP_TYPE_EMPTY) {
            s3k_dump_cap(buf, 256, cap);
            kprintf("%3d: %s\n", i, buf);
        }
    }
    lock_release(&lock);
}

int get_supervisor_cap(uint64_t pid)
{
    cap_t cap;
    for (int i = 0; i < N_CAPS; i++) {
        s3k_read_cap(i, &cap);
        if (cap_is_type(cap, CAP_TYPE_SUPERVISOR) && cap_supervisor_get_free(cap) <= pid &&
            pid <= cap_supervisor_get_end(cap))
            return i;
    }
    return -1;
}

int get_time_cap(uint64_t hartid)
{
    cap_t cap;
    for (int i = 0; i < N_CAPS; i++) {
        s3k_read_cap(i, &cap);
        if (cap_is_type(cap, CAP_TYPE_TIME) && cap_time_get_hartid(cap) == hartid)
            return i;
    }
    return -1;
}

int get_free_slot(void)
{
    cap_t cap;
    for (int i = 0; i < N_CAPS; i++) {
        s3k_read_cap(i, &cap);
        if (cap_is_type(cap, CAP_TYPE_EMPTY))
            return i;
    }
    return -1;
}

bool init_proc(uint64_t pid)
{
    kprintf("Proc %d initializing.\n", pid);
    cap_t cap;
    int sup_index = get_supervisor_cap(0);
    int time_index = get_time_cap(1);
    int empty_index = get_free_slot();
    if (sup_index < 0 || time_index < 0 || empty_index < 0)
        return false;
    s3k_read_cap(time_index, &cap);
    uint64_t free = cap_time_get_free(cap);
    /* May be preempted */
    while (s3k_derive_cap(time_index, empty_index, cap_mk_time(1, free, free + 16, free)))
        ;
    /* May be preempted */
    while (s3k_supervisor_give_cap(sup_index, pid, empty_index, 1))
        ;
    s3k_supervisor_write_reg(sup_index, pid, 0, (uint64_t)user_code);
    s3k_supervisor_write_reg(sup_index, pid, 10, pid);
    kprintf("Proc %d ready.\n", pid);
    return true;
}

void main_supervisor(uint64_t pid, uint64_t begin, uint64_t end)
{
    uart_init();
    for (int i = 1; i <= 5; i++)
        init_proc(i);
    int sup_index = get_supervisor_cap(0);
    kprintf("Starting processes uart_in, app1, app2, and uart_out\n");
    for (int i = 1; i <= 5; i++)
        s3k_supervisor_resume(sup_index, i);
    s3k_yield();
}

void main_uart_in(uint64_t pid, uint64_t begin, uint64_t end)
{
    kprintf("Starting process uart_in\n");
    for (int i = 0; i < ARRAY_SIZE(uart_input); i++)
        uart_input[i] = 0;
    while (1) {
        for (int i = 0; i < ARRAY_SIZE(uart_input); i++)
            uart_input[i] = i + cntr;
        cntr++;
        s3k_yield();
    }
}

void main_uart_out(uint64_t pid, uint64_t begin, uint64_t end)
{
    kprintf("Starting process uart_out\n");
    int k = 0;
    while (1) {
        if (k == cntr)
            continue;
        k = cntr;
        char buf[MSG_SIZE + 16];
        snprintf(buf, ARRAY_SIZE(buf), "0:%d %s\n", k, uart_output[0]);
        puts(buf);
        snprintf(buf, ARRAY_SIZE(buf), "1:%d %s\n", k, uart_output[1]);
        puts(buf);
        s3k_yield();
    }
}

void main_app0(uint64_t pid, uint64_t begin, uint64_t end)
{
    kprintf("Starting process app0\n");
    int k = 0;
    while (1) {
        if (k == cntr)
            continue;
        k = cntr;
        app_output[0][MSG_SIZE] = '\0';
        for (int i = 0; i < MSG_SIZE; i++) {
            int in = uart_input[i];
            int k = in % 16;
            if (k < 10)
                app_output[0][i] = '0' + k;
            else
                app_output[0][i] = 'A' + k - 10;
        }
        s3k_yield();
    }
}

void main_app1(uint64_t pid, uint64_t begin, uint64_t end)
{
    kprintf("Starting process app1\n");
    int k = 0;
    while (1) {
        if (k == cntr)
            continue;
        k = cntr;
        app_output[1][MSG_SIZE] = '\0';
        for (int i = 0; i < MSG_SIZE; i++) {
            int in = uart_input[i];
            int k = in % 16;
            if (k < 10)
                app_output[1][i] = '0' + k;
            else
                app_output[1][i] = 'A' + k - 10;
        }
        s3k_yield();
    }
}

void main_crypt(uint64_t pid, uint64_t begin, uint64_t end)
{
    kprintf("Starting process crypt\n");
    //uint64_t key[2] = {0xdeadbeefdeadbeef, 0xCAFEBABEDEADC0DE};
    while (1) {
        for (int i = 0; i < 2; i++)
            for (int j = 0; j < MSG_SIZE; j++)
                uart_output[i][j] = app_output[i][j];
        s3k_yield();
    }
}
