#include <Arduino.h>
#include <TM1637Display.h> // Install LM1637 by Avishay Orpaz from Arduno library manager or https://github.com/avishorp/TM1637



void setTM1637dec(int pin, int val) {
//  pcSerial.println("TM1673");
  TM1637Display display(pin, pinsExtra[pin]);
  display.setBrightness(0x0f);
  display.showNumberDec(val, false); // Expect: ___0
}
