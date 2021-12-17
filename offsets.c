// See LICENSE file for copyright and license details.

#define OFFSET(NAME, STRUCT, MEMBER) \
        __asm__("#define "#NAME" %0\n":: \
                        "i"(__builtin_offsetof(struct STRUCT, MEMBER)))

struct test {
        long var0;
        char var1, var2;
};

void dummy(void) {
        OFFSET(TEST_VAR0, test, var0);
        OFFSET(TEST_VAR1, test, var1);
        OFFSET(TEST_VAR2, test, var2);
}
