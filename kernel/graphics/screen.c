
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

static va_list* kPrintfArgPtr;
static void (*kPrintfSpecifiersFuncs[31])();


// -- kPrintf and subsequent functions -- 


int kPrintf(const char* stringPtr, ...) {
	va_list argsList;
	va_start(argsList, stringPtr);
	kPrintfArgPtr = &argsList;

	char* curChar;
	char specifier;
	size_t charX, charY;

	for (size_t i = 0; stringPtr[i]; i++) {
		if (textPosX + fontCharWidth > framebufferWidth) {
			textPosX = 0;
			textPosY += fontCharHeight;
		}

		curChar = font8x16 + (stringPtr[i] * fontCharHeight);

		// 0x25 is "%"
		if (stringPtr[i] == 0x25) {
			i++;
			specifier = stringPtr[i];

			if (specifier == 0x25) goto kPrintfCharPrintLoop;
			if (specifier < 0x41 || specifier > 0x7A) continue; // If not a letter ignore it

			// If a lowercase letter
			if (specifier > 0x60) {
				// Execute the function associated to a letter. Eg: "a" -> kPrintfSpecifiersFuncs[0]()
				kPrintfSpecifiersFuncs[specifier - 0x61]();

			} if (specifier < 0x5B) {
				// I could do the same thing for uppercase letters but it would be a waste of memory for only 5 of them being used
				switch (specifier) {
					case 0x41: kPrintfSpecifiersFuncs[26](); break; // "A"
					case 0x45: kPrintfSpecifiersFuncs[27](); break; // "E"
					case 0x46: kPrintfSpecifiersFuncs[28](); break; // "F"
					case 0x47: kPrintfSpecifiersFuncs[29](); break; // "G"
					case 0x58: kPrintfSpecifiersFuncs[30](); break; // "X"

					default: continue;
				}
			}

		} else if (stringPtr[i] == 0x20) {
			textPosX += fontCharWidth;
			continue;
		
		} else if (stringPtr[i] == 0xA) {
			textPosX = 0;
			textPosY += fontCharHeight;
			continue;

		} else {
			kPrintfCharPrintLoop:
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

static void kPrintfC(void) {
	char* arg = va_arg(*kPrintfArgPtr, char*);
	char* curChar = font8x16 + (arg[0] * fontCharHeight);
	
	size_t charX, charY;
	for (charY = 0; charY < fontCharHeight; charY++) {
		for (charX = 0; charX < fontCharWidth; charX++) {
			// If a pixel is present in both curChar and the mask then display it
			if (curChar[charY] & fontCharMask[charX]) framebufferPtr[(textPosX + charX) * framebufferBpp + (textPosY + charY)* framebufferPitch] = screenForegroundColour;
		}
	}
	
	textPosX += fontCharWidth;
}

static void kPrintfS(void) {
	char* arg = va_arg(*kPrintfArgPtr, char*);
	char* curChar;
	size_t i, charX, charY;

	for (i = 0; arg[i]; i++) {
		// Check if we can write the char
		if (textPosX + fontCharWidth > framebufferWidth) {
			textPosX = 0;
			textPosY += fontCharHeight;
		}

		// 8 by 16 bits = 16 bytes per char so every byte describes a row
		curChar = font8x16 + (arg[i] * fontCharHeight);

		for (charY = 0; charY < fontCharHeight; charY++) {
			for (charX = 0; charX < fontCharWidth; charX++) {
				// If a pixel is present in both curChar and the mask then display it
				if (curChar[charY] & fontCharMask[charX]) framebufferPtr[(textPosX + charX) * framebufferBpp + (textPosY + charY)* framebufferPitch] = screenForegroundColour;
			}
		}

		textPosX += fontCharWidth;
	}
}

static void kPrintfPlaceholder(void) {
	return;
}

// Since the functions associated to the specifiers are defined above I can't assign them where i declared this array
static void (*kPrintfSpecifiersFuncs[31])() = {
	kPrintfPlaceholder, kPrintfPlaceholder, kPrintfC, kPrintfPlaceholder, kPrintfPlaceholder, kPrintfPlaceholder, kPrintfPlaceholder, kPrintfPlaceholder,
	kPrintfPlaceholder, kPrintfPlaceholder, kPrintfPlaceholder, kPrintfPlaceholder, kPrintfPlaceholder, kPrintfPlaceholder,
	kPrintfPlaceholder, kPrintfPlaceholder, kPrintfPlaceholder, kPrintfPlaceholder, kPrintfS, kPrintfPlaceholder,
	kPrintfPlaceholder, kPrintfPlaceholder, kPrintfPlaceholder, kPrintfPlaceholder, kPrintfPlaceholder, kPrintfPlaceholder,

	kPrintfPlaceholder, // "A"
	kPrintfPlaceholder, // "E"
	kPrintfPlaceholder, // "F"
	kPrintfPlaceholder, // "G"
	kPrintfPlaceholder // "X"
};


// -- Other text output functions --


void kPrintChar(const char c, uint32_t posX, uint32_t posY) {
	size_t charX, charY = 0;
	char* curChar = font8x16 + (c * fontCharHeight);

	for (charY; charY < fontCharHeight; charY++) {
		for (charX = 0; charX < fontCharWidth; charX++) {
			// If a pixel is present in both curChar and the mask then display it
			if (curChar[charY] & fontCharMask[charX]) framebufferPtr[(posX + charX) * framebufferBpp + (posY + charY)* framebufferPitch] = screenForegroundColour;
		}
	}

}


void kPrint(const char* stringPtr, uint32_t posX, uint32_t posY) {
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
		curChar = font8x16 + (stringPtr[i] * fontCharHeight);

		for (charY = 0; charY < fontCharHeight; charY++) {
			for (charX = 0; charX < fontCharWidth; charX++) {
				// If a pixel is present in both curChar and the mask then display it
				if (curChar[charY] & fontCharMask[charX]) framebufferPtr[(x + charX) * framebufferBpp + (y + charY)* framebufferPitch] = screenForegroundColour;
			}
		}

		x += fontCharWidth;
	}

}


// -- Functions to manage framebuffers ---


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

	screenBackgroundColour = defScreenBackgroundColour;
	screenForegroundColour = defScreenForegroundColour;
	screenAccentColour = defScreenAccentColour;
}