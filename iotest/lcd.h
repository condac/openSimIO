#ifdef LCD

#include <LiquidCrystal.h>

LiquidCrystal lcd(12, 11, 5, 6, 3, 2);

#define LCD_MAX_REFRESH 500

long lcd_refreshtime;

void setupLCD() {
  lcd.begin(16, 2);
  lcd.clear();
  lcd.print("openSimIO");
}

void setupLCDpin(int inPin) {
  // Nothing to do...
}


void lcdLoop() {

  if (millis()>lcd_refreshtime) {
    lcd_refreshtime = millis() + LCD_MAX_REFRESH; 

    lcd.clear();
    lcd.setCursor(0, 0);

    lcd.print("Val1=");
    lcd.print(pinsData[2]);

    lcd.print("Val2=");
    lcd.print(pinsData[3]);
    
  }
  
}




#endif
