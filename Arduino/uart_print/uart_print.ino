#include <ble_mini.h>

void setup()
{
  BLEMini_begin(57600);
}

void loop()
{
<<<<<<< HEAD
  
  bool printingRawData = false;
  // If data is ready
  while (BLEMini_available())
  {
    byte b = BLEMini_read();
    if(b == 0xFF) {
      // end of printing raw data
      if(printingRawData) {
        Serial.print('\n');
      }
      printingRawData = !printingRawData;
    } else if(printingRawData) {
      Serial.print(b);
    } else {
      char c = (char) b;
      Serial.print(c);
    }
  }
  
  if(i>0){
    Serial.print("\n");
  }

  delay(100);
}

