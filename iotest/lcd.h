#ifdef LCD

#include <LiquidCrystal.h>

const int rs = 12, en = 11, d4 = 5, d5 = 6, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

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

// ########################## Modify you text in the LCD Display here #######################
// We can use pin 2 3 5 6 11 12 to transfer data in int format that we can display. 

    lcd.print("Val1=");
    lcd.print(pinsData[2]);

    lcd.print("Val2=");
    lcd.print(pinsData[3]);

    lcd.setCursor(0, 0);
    
    lcd.print("Val3=");
    lcd.print(pinsData[5]);

    lcd.print("Val4=");
    lcd.print(pinsData[6]);        
  }
  
}




#endif
