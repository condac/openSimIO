// Here we have the code to handle the configuration of the pins in the running loop. 
// We look how the pins are configured and we call a function based on the configuration on the pin
// All types of configurations a pin can have is described in "iotypes.h"
// Not sure if pinHandle is best English, send me an issue on github if you know better name ;)

int pinsConfig[DIGITAL_PIN_COUNT+ANALOG_PIN_COUNT]; // this array keeps the configuration a pin have
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
  currentState = (pinsData[pin]*(filter+1) + currentState)/(filter+2);
  
  if ( (currentState < pinsData[pin]-deadband ) || (currentState > pinsData[pin]+deadband) ) {
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

void handlePins(int pinArray[], int numberOfPins) {

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
    #ifdef ROTARY_ENCODER
    case DI_ROTARY_ENCODER_TYPE1:    // same as with pullup, only setup initiation is different
      getRotationType1(i, i+1); // getRotation returns true if changed
      break;
    #endif
    case DI_3WAY_2:    // 
      read3way_2(i, pinsExtra[i]);
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

void setupPins(int configArray[], int numberOfPins) {
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
    }
  }
}
void setupAllPins() {
  setupPins(pinsConfig, DIGITAL_PIN_COUNT+ANALOG_PIN_COUNT);
}
void setupDigitalPins() {
  setupPins(pinsConfig, DIGITAL_PIN_COUNT+ANALOG_PIN_COUNT);
}


void setValue(int pin, int val) {
  switch (pinsConfig[pin]) {
    case 0:    // not configured
      // do nothing
      break;
    case DO_BOOL:    // 
      digitalWrite(pin,val);
      break;
    
    }
  
}
