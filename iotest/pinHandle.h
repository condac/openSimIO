// Here we have the code to handle the configuration of the pins in the running loop. 
// We look how the pins are configured and we call a function based on the configuration on the pin
// All types of configurations a pin can have is described in "iotypes.h"
// Not sure if pinHandle is best English, send me an issue on github if you know better name ;)

int pinsConfig[DIGITAL_PIN_COUNT]; // this array keeps the configuration a pin have
int a_pinsConfig[ANALOG_PIN_COUNT]; // this array keeps the configuration a pin have


void checkPinChanged( int pin) {
  bool currentState = digitalRead(pin);
  if (currentState != pinsData[pin]) {
    pin_changed[pin] = true;
    pinsData[pin] = currentState;
  }
    
}

void readAnalogPinRaw( int pin) {
  
  int currentState = analogRead(pin-DIGITAL_PIN_COUNT);
  if (currentState != pinsData[pin]) {
    pin_changed[pin] = true;
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
    pin_changed[pin] = true;
    pinsData[pin] = currentState;
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
    }
  }
  
}
void handleDigitalPins() {

  handlePins(pinsConfig, DIGITAL_PIN_COUNT+ANALOG_PIN_COUNT);
  
}


void handleAnalogPins() {
  // not used, digital and analog pins now share arrays
  handlePins(a_pinsConfig, ANALOG_PIN_COUNT);
  
}

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
    case DO_HIGH:    // just set the pin to 5v
      pinMode(i, OUTPUT);
      digitalWrite(i,HIGH);
      break;
    case DO_LOW:    // just set the pin to GND 
      pinMode(i, OUTPUT);
      digitalWrite(i,LOW);
      break;          
    case DI_ROTARY_ENCODER_TYPE1:    // same as with pullup, only setup initiation is different
      pinMode(i, INPUT_PULLUP);
      pinMode(i+1, INPUT_PULLUP);
      configArray[i+1] = NOTUSED;
      break;
    
    }
  }
}

void setupDigitalPins() {
  setupPins(pinsConfig, DIGITAL_PIN_COUNT+ANALOG_PIN_COUNT);
}
