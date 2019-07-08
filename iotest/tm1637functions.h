#include <Arduino.h>
#include <TM1637Display.h>



void setTM1637dec(int pin, int val) {
//  pcSerial.println("TM1673");
  TM1637Display display(pin, pinsExtra[pin]);
  display.setBrightness(0x0f);
  display.showNumberDec(val, false); // Expect: ___0
}
