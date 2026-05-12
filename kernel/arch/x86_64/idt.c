
#include <stdint.h>
#include <arch/x86_64/include/gdt.h>
#include <arch/x86_64/include/idt.h>


__attribute__((aligned(0x10))) static idtEntry_t idt[idtEntries];
extern void* isrStubTable[];
extern void isrSetStubTable(void);

static const char* isrExceptionsNames[] = {
    "Division by 0",
    "Debug exception",
    "Non-maskable interrupt exception",
    "Breakpoint exception",
    "Overflow",
    "Bound range exceeded",
    "Invalid opcode",
    "Device not avaiable",
    "Double fault",
    "Coprocessor segment overrun",
    "Invalid TSS",
    "Segment not present",
    "Stack-segment fault",
    "General protection fault",
    "Page fault",
    "Reserved exception",
    "x87 floating-point exception",
    "Alignment check",
    "Machine check",
    "SIMD floating-point exception",
    "Virtualization exception",
    "Control protection exception",
    "Reserved exception",
	"Reserved exception",
	"Reserved exception",
	"Reserved exception",
	"Reserved exception",
	"Reserved exception",
    "Hypervisor injection exception",
    "VMM communication exception",
    "Security exception",
    "Reserved exception"
};


void isrExceptionHandler(intStackFrame_t* stackFrame) {
	asm volatile("cli; hlt");

	// TODO: implement stuff to display a message
}


static void idtSetEntry(uint8_t i, uint64_t* isrAddr, uint8_t ist, uint8_t flags) {
	idt[i].isr0 = (uint16_t)((uint64_t)isrAddr & 0xFFFF);
	idt[i].kernelCs = kernelCodeSeg;
	idt[i].ist = ist;
	idt[i].flags = flags;
	idt[i].isr1 = ((uint64_t)isrAddr >> 16) & 0xFFFFFFFFFFFF;
	idt[i].reserved = 0;
}


void idtInit(void) {
	idtIdtr_t idtr;

	isrSetStubTable();

	for (uint8_t i = 0; i < 32; i++) {
		idtSetEntry(i, isrStubTable[i], 0, 0x8E);
	}

	idtr.size = (uint16_t)sizeof(idt) - 1;
	idtr.offset = (uint64_t)&idt;

	asm volatile("lidt %0" : : "m"(idtr));
}