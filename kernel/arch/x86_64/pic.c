
#include <arch/x86_64/include/pic.h>



void picDisable(void) {
	// Mask every interrupt
	outb(pic1Data, 0xFF);
	outb(pic2Data, 0xFF);

}