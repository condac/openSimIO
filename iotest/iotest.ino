// ############
// User configurations
// ############


// Uncomment the master or slave define, never both!
#define MASTER
//#define SLAVE

#define MASTER_ID 13 // Change if you have more than one master board in your system

#define PC_ID 99 // do not touch

// Enable or disable the serial chain function. 
#define SERIAL_CHAIN


#define FRAMERATE 4

// ############
// Plugins
// ############  IMPORTANT!!!! #########
// To save program memory we only include functions that you know you will use. 
// Uncomment the define statement for each plugin you need in your hardware

// Use rotary encoders
#define ROTARY_ENCODER



// ############
// End of plugins
// ############


#include "boards.h"


String type = BOARD;
//#define TIME_DEBUG 


#include <avr/wdt.h> // Watchdog interupt // 20 bytes for setup and 2 bytes for each reset, no memory

int pinsData[DIGITAL_PIN_COUNT+ANALOG_PIN_COUNT]; 

bool pin_changed[DIGITAL_PIN_COUNT+ANALOG_PIN_COUNT];


#include "iotypes.h"

#ifdef ROTARY_ENCODER
#include "rotaryEncoder.h"
#endif

#include "pinHandle.h"



long frametime;
long pingtime;
#define PING_INTERVAL 1000

#ifdef TIME_DEBUG
long looptime;
long ditime;
long aitime;
long serialtime;
long chaintime;
long debugtime;
#endif

int myId = 0; // this will automaticly be set by the chain ping loop


void readConfig() {
  // Save input/output config in flash memory and read it on boot


  // First read config version string to see if it is compatible
  
}

void setup() {
  wdt_enable(WDTO_1S); //Setup watchdog timeout of 1s. 
  Serial.begin(115200);
  wdt_reset(); //2 bytes
  pcSerial.begin(115200);
  wdt_reset();
  chainSerial.begin(115200);
  wdt_reset();

  // temporary hardcoded setups
  pinsConfig[0] = NOTUSED;
  pinsConfig[1] = NOTUSED;
  pinsConfig[3] = DO_LOW;
  pinsConfig[4] = DO_HIGH;
  pinsConfig[5] = DI_INPUT_PULLUP;
  pinsConfig[6] = DI_ROTARY_ENCODER_TYPE1;
  pinsConfig[7] = NOTUSED;
  pinsConfig[DIGITAL_PIN_COUNT+0] = AI_RAW;
  pinsConfig[DIGITAL_PIN_COUNT+1] = AI_RAW;
  pinsConfig[DIGITAL_PIN_COUNT+2] = AI_FILTER;
  

  pcSerial.println("boot");
  wdt_reset();
  setupDigitalPins();
}

void loop() {
  wdt_reset();
                              #ifdef TIME_DEBUG
                              looptime = millis();
                              #endif
  handleDigitalPins();

                              #ifdef TIME_DEBUG
                              ditime = millis() - looptime;
                              aitime = millis();
                              #endif
  
  
                              #ifdef TIME_DEBUG
                              aitime = millis() - aitime;
                              serialtime = millis();
                              #endif
  if (millis()>frametime) {
    frametime = millis() + (1000/FRAMERATE);
    sendData();
  }

  if (millis()>pingtime) {
    pingtime = millis() + PING_INTERVAL; 
    ping();
  }
                                #ifdef TIME_DEBUG
                                Serial.flush();
                                serialtime = millis() - serialtime;
                                chaintime = millis();
                                #endif
  parseSerial();
  #ifdef MASTER
  parsePCSerial();
  #endif
                                #ifdef TIME_DEBUG
                                Serial.flush();
                                chaintime = millis() - chaintime;
                                #endif
            
            
                                #ifdef TIME_DEBUG
                                      if (millis()>debugtime) {
                                        debugtime = millis() + (1000); 
                                        
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
                                #endif
  //delay(10);
}

void ping() {
  #ifdef MASTER
  chainSerial.print("{100;}");
  chainSerial.print(MASTER_ID);
  chainSerial.println(";}");
  #endif
}
/*
void checkAnalogPinChanged( int pin) {
  int currentState = analogRead(pin);
  if (currentState != a_pins[pin]) {
    a_pin_changed[pin] = true;
    a_pins[pin] = currentState;
  }
    
}*/

void sendData() {
  dataSerial.print("{99;");
  #ifdef MASTER
  dataSerial.print(MASTER_ID);
  dataSerial.print(";");
  #endif
  dataSerial.print(myId);
  dataSerial.print(";");
  for (int i = 0; i<DIGITAL_PIN_COUNT; i++) {
    if (pin_changed[i]) {
      pin_changed[i] = false;
      dataSerial.print("D");
      dataSerial.print(i);
      dataSerial.print(" ");
      dataSerial.print(pinsData[i]);
      dataSerial.print(",");
    }
  }
  for (int i = DIGITAL_PIN_COUNT; i<DIGITAL_PIN_COUNT+ANALOG_PIN_COUNT; i++) {
    if (pin_changed[i]) {
      pin_changed[i] = false;
      dataSerial.print("A");
      dataSerial.print(i-DIGITAL_PIN_COUNT);
      dataSerial.print(" ");
      dataSerial.print(pinsData[i]);
      dataSerial.print(",");
    }
  }
  dataSerial.print(";");
  dataSerial.print(type);
  dataSerial.println("}");
}

void parseSerial() {
  // Check if the serial data is for me
  // if not send it to next device in chain
  while (chainSerial.available() > 0) {
    if (chainSerial.read() == '{') {
      int id = chainSerial.parseInt();
      if ( id == 1 ) {
        // data is for me
        doSomething();
        return;
      } 
      else if (id > 99) {
        // broadcast ping
        // Ping start at 100
        #ifdef SLAVE
        myId = id - 100 + 1;
        // relaySerial removes 1 from id so we add 2 before we send it forward
        id = id + 2;
        relaySerial(id);
        #endif

        #ifdef MASTER
        pcSerial.print("Number of slaves: ");
        pcSerial.println(id-100);
        
        #endif
        return;
      }
      else if ( id == 99 ) {
        // Message to PC/Master 
        relayToPC();
        return;
      }
      else if ( id > 1 ) {
        // relay data forward
        relaySerial(id);
        return;
      } 
      
      else  {
        // id not found?
        pcSerial.println("error else id");
        relayToPC();
        return;
      }
    }
  }

}

#ifdef MASTER
void parsePCSerial() {
  // Check if the serial data is for me
  // if not send it to next device in chain
  while (pcSerial.available() > 0) {
    if (pcSerial.read() == '{') {
      
      int masterid = pcSerial.parseInt();
      char data = pcSerial.read(); // read the ; character
      int slaveid = pcSerial.parseInt();
      
      if (masterid == MASTER_ID) {
        // data for me or my slaves
      
        if ( slaveid == 0 ) {
          // data is for me
          doSomething();
          return;
        } 
        else if ( slaveid > 0 ) {
          // relay data forward
          relaySerialFromPC(slaveid);
          return;
        } 
        else  {
          // id not found?
          pcSerial.println("error else id");
          
          return;
        }
      }
      else if (masterid > 0) {
        // relay to other masters???
        
      }
      else {
        // broadcast?
      }
    }
  }

}
#endif

void relaySerial(int id) {
  // relay data forward
  char data;
  
  chainSerial.print("{");
  id = id - 1;
  chainSerial.print(id);

  while (chainSerial.available() <= 0) {}
  data = chainSerial.read();
  while (data != '}') {
    chainSerial.print(data);
    
    data = chainSerial.read();
    
  }
  // end of message
  chainSerial.println(data);
}


#ifdef MASTER
void relaySerialFromPC(int id) {
  // relay data forward
  char data;
  pcSerial.print("replaying message to slaves");
  chainSerial.print("{");
  id = id;
  chainSerial.print(id);
  while (pcSerial.available() <= 0) {}
  data = pcSerial.read();
  while (data != '}') {
    chainSerial.print(data);
    
    data = pcSerial.read();
    
  }
  // end of message
  chainSerial.println(data);
  
}
#endif

void doSomething() {
  // message for me
  char data;
  
  pcSerial.print("message for me: ");
  while (chainSerial.available() <= 0) {}
  data = chainSerial.read();
  while (data != '}') {
    pcSerial.print(data);
    while (chainSerial.available() <= 0) {
      
    }
    data = chainSerial.read();
    
  }
  // end of message
  pcSerial.println(data);
}

void relayToPC() {
  // message to PC from slave
  char data;
  
  pcSerial.print("{99;");
  pcSerial.print(MASTER_ID); // add master id
  pcSerial.print(""); 
  while (chainSerial.available() <= 0) {}
  data = chainSerial.read();
  while (data != '}') {
    pcSerial.print(data);
    while (chainSerial.available() <= 0) {
      
    }
    data = chainSerial.read();
  }
  // end of message
  pcSerial.println(data);
  return;

}
