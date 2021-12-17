// Here we have the code to handle the configuration of the pins in the running loop. 
// We look how the pins are configured and we call a function based on the configuration on the pin
// All types of configurations a pin can have is described in "iotypes.h"
// Not sure if pinHandle is best English, send me an issue on github if you know better name ;)

long analogFilter[ANALOG_PIN_COUNT+1][11];
int analogFilter2[ANALOG_PIN_COUNT];

void checkPinChanged( int pin) {
  bool currentState = !digitalRead(pin);
  if (currentState != pinsData[pin]) {
    pin_changed[pin] = CHANGE_COUNT;
    pinsData[pin] = currentState;
  }
}

void readAnalogPinRaw( int pin) {
  
  int currentState = analogRead(pin-DIGITAL_PIN_COUNT);
  
  if (currentState != pinsData[pin]) {
    pin_changed[pin] = CHANGE_COUNT;
    pinsData[pin] = currentState;
  }
}

void readAnalogPinFilter( int pin) {
  // Use average filter and a little deadband
  int deadband = 10;
  int filter = 5;
  int currentState = analogRead(pin-DIGITAL_PIN_COUNT);
  analogFilter[pin-DIGITAL_PIN_COUNT][analogFilter2[pin-DIGITAL_PIN_COUNT]] = currentState;
  long sum = 0;
  for (int i=0;i<10;i++) {
    int test = analogFilter[pin-DIGITAL_PIN_COUNT][i];
    sum = sum + test;
//    Serial.print(",");
//    Serial.print(test);
  }
  analogFilter2[pin-DIGITAL_PIN_COUNT]++;
  if (analogFilter2[pin-DIGITAL_PIN_COUNT] > 10) {
    analogFilter2[pin-DIGITAL_PIN_COUNT] = 0;
  }
  
  sum = sum /10;
//  Serial.print("sum ");
//  Serial.print(sum);
//  Serial.print("currentState ");
//  Serial.println(currentState);
  currentState = sum;
  
  if ( (currentState < pinsExtra[pin]-deadband ) || (currentState > pinsExtra[pin]+deadband) ) {
    pin_changed[pin] = CHANGE_COUNT;
    pinsExtra[pin] = currentState;
  }
  pinsData[pin] = currentState;
}
void readAnalogPinOverSample( int pin) {
  
  int currentState = analogRead(pin-DIGITAL_PIN_COUNT);
  
  analogFilter[pin-DIGITAL_PIN_COUNT][analogFilter2[pin-DIGITAL_PIN_COUNT]] = currentState;
  long sum = 0;
  for (int i=0;i<10;i++) {
    int test = analogFilter[pin-DIGITAL_PIN_COUNT][i];
    sum = sum + test;
//    Serial.print(",");
//    Serial.print(test);
  }
  analogFilter2[pin-DIGITAL_PIN_COUNT]++;
  if (analogFilter2[pin-DIGITAL_PIN_COUNT] > 10) {
    analogFilter2[pin-DIGITAL_PIN_COUNT] = 0;
  }
  
  //sum = sum /10;
//  Serial.print("sum ");
//  Serial.print(sum);
//  Serial.print("currentState ");
//  Serial.println(currentState);
  currentState = sum;
  if (currentState != pinsData[pin]) {
    pin_changed[pin] = CHANGE_COUNT;
    pinsData[pin] = currentState;
  }
}

void read3way_2( int pin, int extra) {
  int newValue = 0;

  // this function is very special for our simulator because of the way it is wired. 
  bool currentState = digitalRead(pin);
  bool currentState2 = digitalRead(extra);
//  pcSerial.print(currentState);
//  pcSerial.print(currentState2);
//  pcSerial.println("3way");
  if (currentState && currentState2) {
    newValue = 0;
  } else if (!currentState && currentState2) {
    newValue = 22;
  } else if (currentState && !currentState2) {
    newValue = 45;
  } else {
    // this should not happen, but it might during transition so we set the middle value
    newValue = 30;
  }
  if (pinsData[pin] != newValue) {
    pinsData[pin] = newValue;
    pin_changed[pin] = CHANGE_COUNT;
  }
  
}
void read4x4(int pin) {
  int newValue = 0;
  // put your main code here, to run repeatedly:
  for (int i = 0; i < 4; i++) {
    bool colSet[4] = {HIGH,HIGH,HIGH,HIGH};
    colSet[i] = LOW;
    digitalWrite(pin+4,colSet[0]);
    digitalWrite(pin+5,colSet[1]);
    digitalWrite(pin+6,colSet[2]);
    digitalWrite(pin+7,colSet[3]);
    colSet[i] = HIGH;
    bool row0 = digitalRead(pin+0);
    bool row1 = digitalRead(pin+1);
    bool row2 = digitalRead(pin+2);
    bool row3 = digitalRead(pin+3);
    
    if(row0 && !row1 ){
          //Serial.print(1+i*4);
          newValue = 1+i*4;
    }else if(row1 && !row2){
          //Serial.print(2+i*4);
          newValue = 2+i*4;
    }else if(row2 && !row3){
          //Serial.print(3+i*4);
          newValue = 3+i*4;
    }else if(row3 && !row0){
          //Serial.print(4+i*4);
          newValue = 4+i*4;
    }else{;}
  }
 
  
  if (pinsData[pin] != newValue) {
    pinsData[pin] = newValue;
    pin_changed[pin] = CHANGE_COUNT;
    Serial.print("4x4: ");
    Serial.println(newValue);
  }
}


void handlePins(uint8_t pinArray[], int numberOfPins) {

  for (int i = 0; i<numberOfPins; i++) {
    switch (pinArray[i]) {
    case 0:    // not configured
      
      break;
    case DI_INPUT_PULLUP:    // 
      checkPinChanged(i);
      break;
    case DI_INPUT_FLOATING:    // same as with pullup, only setup initiation is different
      checkPinChanged(i);
      break;
    case DI_INPUT_STEP:    // same as with pullup, handled in plugin software
      checkPinChanged(i);
      break;
    case AI_RAW:    // Analog raw value
      readAnalogPinRaw(i);
      break;
    case AI_FILTER:    // Analog filtered value
      readAnalogPinFilter(i);
      break;
    case AI_OVERSAMPLE:    // Analog 10 times and add the sum value
      readAnalogPinOverSample(i);
      break;
    #ifdef ROTARY_ENCODER
    case DI_ROTARY_ENCODER_TYPE1:    // same as with pullup, only setup initiation is different
      getRotationType1(i, i+1); // getRotation returns true if changed
      break;
    #endif
    case DI_3WAY_2:    // 
      read3way_2(i, pinsExtra[i]);
      break;
      
    case DI_4X4:    // 
      read4x4(i);
      break;
    }
  }
  
}
void handleDigitalPins() {

  handlePins(pinsConfig, DIGITAL_PIN_COUNT+ANALOG_PIN_COUNT);

}


/*void handleAnalogPins() {
  // not used, digital and analog pins now share arrays
  handlePins(a_pinsConfig, ANALOG_PIN_COUNT);
  
}*/

void setupPins(uint8_t configArray[], int numberOfPins) {
  for (int i = 0; i<numberOfPins; i++) {
    switch (configArray[i]) {
    case 0:    // not configured
      // do nothing
      break;
    case DI_INPUT_PULLUP:    // 
      pinMode(i, INPUT_PULLUP);
      break;
    case DI_INPUT_FLOATING:    // same as with pullup, only setup initiation is different
      pinMode(i, INPUT);
      break;
    case DI_INPUT_STEP:    // 
      pinMode(i, INPUT_PULLUP);
      break;
    case AI_RAW:    // 
      pinMode(i, OUTPUT);
      digitalWrite(i,LOW);
      pinMode(i, INPUT);
      break;
    case AI_FILTER:    // 
      pinMode(i, OUTPUT);
      digitalWrite(i,LOW);
      pinMode(i, INPUT);
      break;
    case AI_OVERSAMPLE:    // 
      pinMode(i, OUTPUT);
      digitalWrite(i,LOW);
      pinMode(i, INPUT);
      break;
    case DO_HIGH:    // just set the pin to 5v
      pinMode(i, OUTPUT);
      digitalWrite(i,HIGH);
      break;
    case DO_LOW:    // just set the pin to GND 
      pinMode(i, OUTPUT);
      digitalWrite(i,LOW);
      break;  
    case DO_BOOL:    // set outputmode 
      pinMode(i, OUTPUT);
      break;  
    case AO_PWM:    // set outputmode 
      pinMode(i, OUTPUT);
      break;
    case DI_ROTARY_ENCODER_TYPE1:    // same as with pullup, only setup initiation is different
      pinMode(i, INPUT_PULLUP);
      pinMode(i+1, INPUT_PULLUP);
      configArray[i+1] = NOTUSED;
      break;
    case DI_3WAY_2:    // 
      pinMode(i, INPUT_PULLUP);
//      pinMode(i+1, OUTPUT);
//      digitalWrite(i+1,LOW);
      
      pinMode(pinsExtra[i], INPUT_PULLUP);
//      pinMode(pinsExtra[i]+1, OUTPUT);
//      digitalWrite(pinsExtra[i]+1,LOW);
      
      configArray[pinsExtra[i]] = NOTUSED;
  
      break;
#ifdef TM1637
    case DO_TM1637_DEC:    // 
      pinMode(i, OUTPUT);
      pinMode(pinsExtra[i], OUTPUT);
      break;
#endif
#ifdef SERVO
    case AO_SERVO:    // 
      setupServo(i);

      break;
#endif
    case DI_4X4:    // 

      //Make row pins input
      pinMode(i, INPUT_PULLUP);
      pinMode(i+1, INPUT_PULLUP);
      pinMode(i+2, INPUT_PULLUP);
      pinMode(i+3, INPUT_PULLUP);
      pinMode(i+4, OUTPUT);
      pinMode(i+5, OUTPUT);
      pinMode(i+6, OUTPUT);
      pinMode(i+7, OUTPUT);
      
      configArray[i+1] = NOTUSED;
      configArray[i+2] = NOTUSED;
      configArray[i+3] = NOTUSED;
      configArray[i+4] = NOTUSED;
      configArray[i+5] = NOTUSED;
      configArray[i+6] = NOTUSED;
      configArray[i+7] = NOTUSED;
      
      break;
#ifdef LCD
    case AO_LCD:    // 
      setupLCDpin(i);
      break;
#endif

#ifdef MCP23017x
    case MCP_DI:    // 
      setupMCPpin(i);
      break;
#endif
    }
  }
}
void setupAllPins() {
  setupPins(pinsConfig, DIGITAL_PIN_COUNT+ANALOG_PIN_COUNT+MCP_PIN_COUNT);
}
void setupDigitalPins() {
  setupPins(pinsConfig, DIGITAL_PIN_COUNT+ANALOG_PIN_COUNT+MCP_PIN_COUNT);
}


void setValue(int pin, int val) {
  switch (pinsConfig[pin]) {
    case 0:    // not configured
      // do nothing
      break;
    case DO_BOOL:    // 
      //pinMode(pin, OUTPUT);
      digitalWrite(pin,val);
      break;
    case AO_PWM:    // 
      analogWrite(pin,val);
      break;
      
#ifdef TM1637
    case DO_TM1637_DEC:    // 
      setTM1637dec(pin,val);
      break;
#endif
#ifdef SERVO
    case AO_SERVO:    // 
      setServo(pin,val);
      break;
#endif
#ifdef STEPPER
    case AO_STEPPER:    // 
      pinsData[pin] = val;
      break;
#endif

#ifdef LCD
    case AO_LCD:    // 
      pinsData[pin] = val;
      break;
#endif

#ifdef MCP23017x
    case MCP_DO:    // 
      setMCPpin(pin, val);
      break;
#endif
    }
}
