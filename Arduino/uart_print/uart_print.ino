#include <ble_mini.h>

void setup()
{
  BLEMini_begin(57600);
}

void loop()
{
  int i = BLEMini_available();
  // If data is ready
  while (BLEMini_available())
  {
    int b = BLEMini_read();
    char c = (char) b;
    // print it out
    Serial.print(b);
    Serial.print(", ");
  }
  if(i>0){
    Serial.print("\n");
    }

  delay(100);
}

