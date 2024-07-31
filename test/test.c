#include <stdint.h>

#define PPC_PUTCHAR 0xa0000000
#define PPC_SYSCON_EXIT 0xe0000000

void syscon_exit(uint32_t code);
int main();

void _start() {
    syscon_exit(main());
}

void syscon_exit(uint32_t code) {
    *((volatile uint8_t*)PPC_SYSCON_EXIT) = code;
}

void putchar(char c) {
    *((volatile uint8_t*)PPC_PUTCHAR) = c;
}

void puts(const char* s) {
    while (*s)
        putchar(*s++);
}

int main() {
    puts("Hello, world!\n");

    return 0xdeadc0de;
}