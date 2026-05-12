
#include <stdint.h>
#include <stddef.h>
#include <graphics/include/screen.h>
#include <limine.h>

static struct limine_framebuffer* defFramebuffer;
static volatile uint32_t* framebufferPtr;
static uint64_t defFramebufferPitch;

__attribute__((used, section(".limine_requests")))
static volatile struct limine_framebuffer_request limineFramebufferRequest = {
	.id = LIMINE_FRAMEBUFFER_REQUEST_ID,
	.revision = 6
};


void screenDrawRectangle(uint32_t posX, uint32_t posY, uint32_t width, uint32_t height, uint32_t colour) {
	if (posX + width > defFramebuffer->width || posY + height > defFramebuffer->height) return;

	uint32_t x;

	for (uint32_t y = posY; y < posY + height; y++) {
		for (x = posX; x < posX + width; x++) {
			framebufferPtr[y * defFramebufferPitch + x] = colour;
		}
	}
}


void screenPaintBackground(uint32_t colour) {
	size_t x;

	for (size_t y = 0; y < defFramebuffer->height; y++) {
		for (x = 0; x < defFramebuffer->width; x++) {
			framebufferPtr[y * defFramebufferPitch + x] = colour;
		}
	}
}


void screenInit(void) {
	if (limineFramebufferRequest.response == NULL || limineFramebufferRequest.response->framebuffer_count < 1) return;

	defFramebuffer = limineFramebufferRequest.response->framebuffers[0];
	framebufferPtr = defFramebuffer->address;
	defFramebufferPitch = defFramebuffer->pitch / 4;

	screenPaintBackground(0x1A1B25);
	screenDrawRectangle(250, 200, 300, 150, 0xFFFFFF);
}