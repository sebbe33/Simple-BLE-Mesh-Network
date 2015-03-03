#include <ble_mini.h>

void setup()
{
  BLEMini_begin(57600);
}

void loop()
{
  
  // If data is ready
  while (BLEMini_available())
  {
    char c = (char) BLEMini_read();
    // print it out
    Serial.print(c);
  }
  delay(100);
}

