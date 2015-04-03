
/*

  Copyright (c) 2013 RedBearLab

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal 
  in the Software without restriction, including without limitation the rights 
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.

*/

#include "i2c.h"
#include "eeprom.h"

void eeprom_write_25(unsigned char *data)
{
    i2c_start(0xa0);
    i2c_write(0x00);  // High byte
    i2c_write(0x00);  // Low byte
    for (int i = 0; i < 25; i++)
      i2c_write(data[i]);
    i2c_stop();
}

void eeprom_write_long(unsigned short address, unsigned long value) 
{
      unsigned char* r;
      unsigned long helper = value;
      r = (unsigned char*) &helper;
      eeprom_page_write(address, r[0], r[1], r[2], r[3]);
}

void eeprom_page_write(unsigned short addr, unsigned char wdata0, unsigned char wdata1, unsigned char wdata2, unsigned char wdata3)
{
    i2c_start(0xa0);
    i2c_write(addr>>8);  // High byte
    i2c_write(addr);  // Low byte
    i2c_write(wdata0);
    for (int i = 0; i < 5000; i++)
      asm("nop");
    i2c_write(wdata1);
    for (int i = 0; i < 5000; i++)
      asm("nop");
    i2c_write(wdata2);
    for (int i = 0; i < 5000; i++)
      asm("nop");
    i2c_write(wdata3);
    i2c_stop();
    
    for (int i = 0; i < 5000; i++)
      asm("nop");
}

void eeprom_write(unsigned short addr, unsigned char wdata)
{
    i2c_start(0xa0);
    i2c_write(addr>>8);  // High byte
    i2c_write(addr);  // Low byte
    i2c_write(wdata);
    i2c_stop();

    for (int i = 0; i < 5000; i++)
      asm("nop");
}

void eeprom_write_bytes(unsigned short addr, unsigned char* data, unsigned char len){
  for(int i=0; i<len; i++){
    eeprom_write(addr+i, data[i]);
  }
}

unsigned char eeprom_read(unsigned short addr)
{
    unsigned char r;
    
    i2c_start(0xa0);
    i2c_write(addr>>8);  // High byte
    i2c_write(addr);  // Low byte
    i2c_restart(0xa1);
    r=i2c_read(1);
    i2c_stop();
    
    return r;    
}

unsigned long eeprom_read_long(unsigned short addr)
{
    unsigned long result;
    unsigned char r[4];
    
    i2c_start(0xa0);
    i2c_write(addr>>8);  // High byte
    i2c_write(addr);  // Low byte
    i2c_restart(0xa1);
    //result |= (i2c_read(1) << 24) & 0xFF000000;
    //result |= 0x01000000;
    r[0] = i2c_read(1);
    i2c_restart(0xa1);
    //result |= (i2c_read(2) << 16) & 0x00FF0000;
    //result |= 0x00020000;
    r[1] = i2c_read(1);
    i2c_restart(0xa1);
    //result |= i2c_read(3) << 8;
    r[2] = i2c_read(1);
    i2c_restart(0xa1);
    //result |= i2c_read(4);
    r[3] = i2c_read(1);
    i2c_stop();
    result = *( (unsigned long*) r);
    return result;    
}

void eeprom_read_bytes(unsigned short addr, unsigned char* data, unsigned char len){
      for(int i=0; i<len; i++){
      data[i] = eeprom_read(addr+i);
  }
}

