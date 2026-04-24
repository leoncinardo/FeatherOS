
#include <arch/x86_64/include/ioPorts.h>


uint8_t inb(uint16_t port) {
    uint8_t input;
    asm volatile("in %%dx, %%al" : "=a" (input) : "d" (port));

    return input;
}


uint16_t inw(uint16_t port) {
	uint16_t input;
	asm volatile("in %%dx, %%ax" : "=a" (input) : "d" (port));

	return input;
}


void outb(uint16_t port, uint8_t byte) {
    asm volatile("out %%al, %%dx" : : "a" (byte), "d" (port));
}


void outw(uint16_t port, uint16_t word) {
    asm volatile("out %%ax, %%dx" : : "a"(word), "d" (port));
}

