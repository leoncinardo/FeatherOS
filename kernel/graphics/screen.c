
#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include <limine.h>
#include <graphics/include/screen.h>
#include <graphics/include/print.h>
#include <graphics/include/font.h>


__attribute__((used, section(".limine_requests")))
static volatile struct limine_framebuffer_request limineFramebufferRequest = {
	.id = LIMINE_FRAMEBUFFER_REQUEST_ID,
	.revision = 6
};


static struct limine_framebuffer* framebuffer = NULL;
static volatile uint32_t* framebufferPtr;
static uint64_t framebufferWidth;
static uint64_t framebufferHeight;
static uint64_t framebufferPitch;
static uint32_t framebufferBpp;

static uint32_t textPosX, textPosY = 0;
static uint32_t screenBackgroundColour, screenForegroundColour, screenAccentColour;

static va_list* klogArgPtr;
static void (*klogSpecifiersFuncs[26])();


// -- klog and subsequent functions -- 


int klog(const char* stringPtr, ...) {
	va_list argsList;
	va_start(argsList, stringPtr);
	klogArgPtr = &argsList;

	char* curChar;
	size_t charX, charY;

	for (size_t i = 0; stringPtr[i]; i++) {
		if (textPosX + fontCharWidth > framebufferWidth) {
			textPosX = 0;
			textPosY += fontCharHeight;
		}

		curChar = g_8x16_font + (stringPtr[i] * fontCharHeight);

		// 0x25 is "%"
		if (stringPtr[i] == 0x25) {
			if (!klogSpecifiersFuncs[i + 1]) continue;
			i++;

			// Execute the function associated to a letter.
			// For example "A" -> klogSpecifiersFuncs[0]()
			if (stringPtr[i] > 0x61) {
				// Lowercase
				klogSpecifiersFuncs[stringPtr[i] - 0x61]();

			} else {
				// Uppercase
				klogSpecifiersFuncs[stringPtr[i] - 0x41]();

			}

		} else {
			for (charY = 0; charY < fontCharHeight; charY++) {
				for (charX = 0; charX < fontCharWidth; charX++) {
					// If a pixel is present in both curChar and the mask then display it
					if (curChar[charY] & fontCharMask[charX]) framebufferPtr[(textPosX + charX) * framebufferBpp + (textPosY + charY)* framebufferPitch] = screenForegroundColour;
				}
			}
		}

		textPosX += fontCharWidth;
	}

	va_end(argsList);
	return 0;
}

static void klogC(void) {
	char arg = va_arg(*klogArgPtr, char);
	char* curChar = g_8x16_font + (arg * fontCharHeight);

	size_t charX, charY;
	for (charY = 0; charY < fontCharHeight; charY++) {
		for (charX = 0; charX < fontCharWidth; charX++) {
			// If a pixel is present in both curChar and the mask then display it
			if (curChar[charY] & fontCharMask[charX]) framebufferPtr[(textPosX + charX) * framebufferBpp + (textPosY + charY)* framebufferPitch] = screenForegroundColour;
		}
	}

	textPosX += fontCharWidth;
}

static void klogS(void) {
	char* arg = va_arg(*klogArgPtr, char*);
	char* curChar;
	size_t i, charX, charY;

	for (i = 0; arg[i]; i++) {
		// Check if we can write the char
		if (textPosX + fontCharWidth > framebufferWidth) {
			textPosX = 0;
			textPosY += fontCharHeight;
		}

		// 8 by 16 bits = 16 bytes per char so every byte describes a row
		curChar = g_8x16_font + (arg[i] * fontCharHeight);

		for (charY = 0; charY < fontCharHeight; charY++) {
			for (charX = 0; charX < fontCharWidth; charX++) {
				// If a pixel is present in both curChar and the mask then display it
				if (curChar[charY] & fontCharMask[charX]) framebufferPtr[(textPosX + charX) * framebufferBpp + (textPosY + charY)* framebufferPitch] = screenForegroundColour;
			}
		}

		textPosX += fontCharWidth;
	}
}

static void klogPlaceholder(void) {
	return;
}

// Since the functions associated to the specifiers are defined above I can't assign them where i declared this array
static void (*klogSpecifiersFuncs[26])() = {
	klogPlaceholder, klogPlaceholder, klogC, klogPlaceholder, klogPlaceholder, klogPlaceholder, klogPlaceholder, klogPlaceholder,
	klogPlaceholder, klogPlaceholder, klogPlaceholder, klogPlaceholder, klogPlaceholder, klogPlaceholder,
	klogPlaceholder, klogPlaceholder, klogPlaceholder, klogPlaceholder, klogS, klogPlaceholder,
	klogPlaceholder, klogPlaceholder, klogPlaceholder, klogPlaceholder, klogPlaceholder, klogPlaceholder
};


// Other text output functions


void kprintChar(const char c, uint32_t posX, uint32_t posY) {
	size_t charX, charY = 0;
	char* curChar = g_8x16_font + (c * fontCharHeight);

	for (charY; charY < fontCharHeight; charY++) {
		for (charX = 0; charX < fontCharWidth; charX++) {
			// If a pixel is present in both curChar and the mask then display it
			if (curChar[charY] & fontCharMask[charX]) framebufferPtr[(posX + charX) * framebufferBpp + (posY + charY)* framebufferPitch] = screenForegroundColour;
		}
	}

}


void kprint(const char* stringPtr, uint32_t posX, uint32_t posY) {
	uint32_t x = posX;
	uint32_t y = posY;
	char* curChar;
	size_t i, charX, charY;

	for (i = 0; stringPtr[i]; i++) {
		// Check if we can write the char
		if (x + fontCharWidth > framebufferWidth) {
			x = 0;
			y += fontCharHeight;
		}

		// 8 by 16 bits = 16 bytes per char so every byte describes a row
		curChar = g_8x16_font + (stringPtr[i] * fontCharHeight);

		for (charY = 0; charY < fontCharHeight; charY++) {
			for (charX = 0; charX < fontCharWidth; charX++) {
				// If a pixel is present in both curChar and the mask then display it
				if (curChar[charY] & fontCharMask[charX]) framebufferPtr[(x + charX) * framebufferBpp + (y + charY)* framebufferPitch] = screenForegroundColour;
			}
		}

		x += fontCharWidth;
	}

}


// Functions to manage framebuffers


void screenDrawRectangle(uint32_t posX, uint32_t posY, uint32_t width, uint32_t height, uint32_t colour) {
	if (posX + width > framebufferWidth || posY + height > framebufferHeight) return;

	uint32_t x, y;

	for (y = posY; y < posY + height; y++) {
		for (x = posX; x < posX + width; x++) {
			framebufferPtr[x * framebufferBpp + y * framebufferPitch] = colour;
		}
	}
}


void screenPaintBackground(uint32_t colour) {
	uint32_t x, y;

	for (y = 0; y < framebufferHeight; y++) {
		for (x = 0; x < framebufferWidth; x++) {
			framebufferPtr[x * framebufferBpp + y * framebufferPitch] = colour;
		}
	}
}


void screenInit(void) {
	if (limineFramebufferRequest.response == NULL || limineFramebufferRequest.response->framebuffer_count < 1) return;

	// We want a framebuffer with 32 bpp
	for (size_t i = 0; i < limineFramebufferRequest.response->framebuffer_count; i++) {
		if (limineFramebufferRequest.response->framebuffers[i]->bpp != 32) continue;

		framebuffer = limineFramebufferRequest.response->framebuffers[i];
		framebufferPtr = framebuffer->address;
		framebufferWidth = framebuffer->width;
		framebufferHeight = framebuffer->height;
		framebufferPitch = framebuffer->pitch / 4;
		framebufferBpp = framebuffer->bpp / 32;
		break;
	}

	if (framebuffer == NULL) return;

	screenBackgroundColour = 0x1A1B25;
	screenForegroundColour = 0xFFFFFF;
	screenAccentColour = 0x67E544;
}