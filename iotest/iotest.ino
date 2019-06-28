// User configurations

#define FRAMERATE 60



// ### We check what kind of arduino board that is used if we need to do somehting different depending on hardware

#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)

//Arduino Mega
  #define PIN_NR 54
  #define ANALOG_PINS 16

  HardwareSerial& chainSerial = Serial1;
  HardwareSerial& pcSerial = Serial;
#endif

#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__)

//Arduino Uno
  #define PIN_NR 14
  #define ANALOG_PINS 6
  HardwareSerial& chainSerial = Serial;
  HardwareSerial& pcSerial = Serial;
#endif


#if defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega16U4__)

//Arduino Leonardo
  #define PIN_NR 14
  #define ANALOG_PINS 6
  HardwareSerial& chainSerial = Serial1;
  Serial_& pcSerial = Serial;
#endif


// Uncomment the master or slave define 
#define MASTER
// #define SLAVE

int pins[PIN_NR];
bool pin_changed[PIN_NR];

int a_pins[PIN_NR];
bool a_pin_changed[PIN_NR];

long looptime;
long ditime;
long aitime;
long serialtime;
long chaintime;

long frametime;

long debugtime;

int myId = 0; // this will automaticly be set by the chain ping loop


void setup() {
  Serial.begin(115200);
  chainSerial.begin(115200);
  // put your setup code here, to run once:
  for (int i = 0; i<PIN_NR; i++) {
    pins[i] = 0;
    pinMode(i, INPUT_PULLUP);
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  looptime = millis();
  
  for (int i = 0; i<PIN_NR; i++) {
    checkPinChanged(i);
  }
  
  ditime = millis() - looptime;
  aitime = millis();
  
  for (int i = 0; i<ANALOG_PINS; i++) {
    checkAnalogPinChanged(i);
  }
  
  aitime = millis() - aitime;
  serialtime = millis();

  if (millis()>frametime) {
    frametime = millis() + (1000/FRAMERATE);
    sendData();
  }
  
  if (millis()>debugtime) {
    debugtime = millis() + (1000);  
    Serial.flush();
    serialtime = millis() - serialtime;
  
    chaintime = millis();
    parseSerial();
    Serial.flush();
  
    chaintime = millis() - chaintime;
    
    looptime = millis() - looptime;
  
    Serial.print("digital:");
    Serial.print(ditime);
    
    Serial.print(" analog:");
    Serial.print(aitime);
    
    Serial.print(" serial:");
    Serial.print(serialtime);
    
    Serial.print(" chain:");
    Serial.print(chaintime);
    
    Serial.print(" loop:");
    Serial.println(looptime);
  
  
    
    Serial.flush();
  }
  //delay(10);
}

void checkPinChanged( int pin) {
  bool currentState = digitalRead(pin);
  if (currentState != pins[pin]) {
    pin_changed[pin] = true;
    pins[pin] = currentState;
  }
    
}
void checkAnalogPinChanged( int pin) {
  int currentState = analogRead(pin);
  if (currentState != a_pins[pin]) {
    a_pin_changed[pin] = true;
    a_pins[pin] = currentState;
  }
    
}

void sendData() {
  Serial.print("{0;");
  Serial.print(myId);
  Serial.print(";");
  for (int i = 0; i<PIN_NR; i++) {
    if (pin_changed[i]) {
      pin_changed[i] = false;
      Serial.print("D");
      Serial.print(i);
      Serial.print(" ");
      Serial.print(pins[i]);
      Serial.print(" ");
    }
  }
  for (int i = 0; i<ANALOG_PINS; i++) {
    if (a_pin_changed[i]) {
      a_pin_changed[i] = false;
      Serial.print("A");
      Serial.print(i);
      Serial.print(" ");
      Serial.print(a_pins[i]);
      Serial.print(" ");
    }
  }
  Serial.println("}");
}

void parseSerial() {
  // Check if the serial data is for me
  // if not send it to next device in chain
  while (chainSerial.available() > 0) {
    if (chainSerial.read() == '{') {
      int id = chainSerial.parseInt();
      if ( id == 1 ) {
        // data is for me
      } else if (id > 99) {
        // broadcast ping
        // Ping start at 100
        myId = id - 100 + 1;
        // relaySerial removes 1 from id so we add 2 before we send it forward
        id = id + 2;
        relaySerial(id);
      }
        else if ( id > 1 ) {
      
        // relay data forward
        relaySerial(id);
        return;
      }
    }
  }

}

void relaySerial(int id) {
  // relay data forward
  char data;
  
  chainSerial.print("{");
  id = id - 1;
  chainSerial.print(id);
  while (chainSerial.available() > 0) {
    data = chainSerial.read();
    if (data == '}') {
      // end of message
      chainSerial.print(data);
      return;
    }
    else {
      chainSerial.print(data);
    }
  }
  // If we are here the message was not ended with '}'
  // TODO Figure out how to handle problem
}
