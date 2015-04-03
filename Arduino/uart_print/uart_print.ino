#include <ble_mini.h>

void setup()
{
  BLEMini_begin(57600);
}
int shortCount = 0, longCount = 0;
void loop()
{
 
   byte longBuf[4] = {0};
   byte shortBuf[2] = {0}; 
  bool printingRawData = false, printingLong = false, printingShort = false;
  // If data is ready
  while (BLEMini_available())
  {
    
    byte b = BLEMini_read();
    if(b == 0xFE && longCount == 0 && shortCount == 0) {
      // end of printing raw data
      if(printingRawData) {
        Serial.print('\n');
      }
      printingRawData = !printingRawData;
    } else if(!printingLong && b == 0xFD) {
      printingShort = true;
    } else if(!printingShort && b == 0xFC) {
      printingLong = true;
    } else if(printingRawData) {
      Serial.print(b);
      Serial.print(",");
    } else if(printingShort) {
        shortBuf[shortCount] = b;
        shortCount++;
        if(shortCount == 2) {
         shortCount = 0;
         printingLong = false;
         Serial.print(*((unsigned short*) shortBuf));
         Serial.print('\n');
       }
    } else if(printingLong) {
      longBuf[longCount] = b;
       longCount++;
       if(longCount == 4) {
         longCount = 0;
         printingLong = false;
         Serial.print(*((unsigned long*) longBuf));
         Serial.print('\n');
       }
    } else {
      char c = (char) b;
      Serial.print(c);
    }
  }

  delay(100);
}

