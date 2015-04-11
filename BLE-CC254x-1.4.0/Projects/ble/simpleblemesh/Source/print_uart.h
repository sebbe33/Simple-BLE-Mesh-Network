#ifndef UART_DEBUG
#define UART_DEBUG
#include "bcomdef.h"

void debugPrint(char* charSequence);

void debugPrintLine(char* charSequence);

void debugPrintRaw(uint8* data);

void debugPrintRawArray(uint8* data, uint8 len);

void debugPrintRaw16(uint16* data);

void debugPrintRaw32(uint32* data);
#endif