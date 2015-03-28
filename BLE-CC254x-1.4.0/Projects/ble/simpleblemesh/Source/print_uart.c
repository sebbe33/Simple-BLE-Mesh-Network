#include "print_uart.h"
#include "hal_uart.h"
#include "npi.h"


static uint16 getLengthOfString(char*);

void debugPrint(char* charSequence)
{
  HalUARTWrite(NPI_UART_PORT, (uint8*)charSequence, getLengthOfString(charSequence)); 
}

void debugPrintLine(char* charSequence)
{
  uint8 newLineChar = '\n';
  HalUARTWrite(NPI_UART_PORT, (uint8*)charSequence, getLengthOfString(charSequence)); 
  HalUARTWrite(NPI_UART_PORT, &newLineChar, 1); 
}

void debugPrintRaw(uint8* data)
{
  uint8 rawinit = 0xFE;
  HalUARTWrite(NPI_UART_PORT, &rawinit, 1); 
  HalUARTWrite (NPI_UART_PORT, data, 1);
  HalUARTWrite(NPI_UART_PORT, &rawinit, 1); 
}

void debugPrintRawArray(uint8* data, uint8 len){
  uint8 rawinit = 0xFE;
  HalUARTWrite(NPI_UART_PORT, &rawinit, 1); 
  HalUARTWrite(NPI_UART_PORT, data, len); 
  HalUARTWrite(NPI_UART_PORT, &rawinit, 1); 

}

static uint16 getLengthOfString(char* charSequence) 
{
  uint16 length = 0;
  for (char* pointerIndex = charSequence; *pointerIndex != '\0'; pointerIndex++){
    length++;
  }
  return length;
}
