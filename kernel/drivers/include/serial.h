
#pragma once

#define COM1 0x3F8
#define COM2 0x2F8

uint8_t serialRead(void);
void serialWrite(uint8_t c);
int serialInit(void);