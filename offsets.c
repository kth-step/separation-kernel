#include <stddef.h>
struct test {
        unsigned int var1, var2;
};

#define OFFSET(NAME, STRUCT, MEMBER) \
        __asm__("#define "#NAME" %0\n"::"i"(offsetof(struct STRUCT, MEMBER)))

void dummy(void) {
        OFFSET(TEST_VAR1, test, var1);
        OFFSET(TEST_VAR2, test, var2);
}
