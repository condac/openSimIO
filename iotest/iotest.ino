
#include "Config.h" // This file contains all user configurations you need to make

#include "boards.h"
#include "common.h"

String type = BOARD;

// Show debug stuff
//#define TIME_DEBUG 


#include <avr/wdt.h> // Watchdog interupt // 20 bytes for setup and 2 bytes for each reset, no memory

uint8_t pinsConfig[DIGITAL_PIN_COUNT+ANALOG_PIN_COUNT+MCP_PIN_COUNT]; // this array keeps the configuration a pin have
int pinsData[DIGITAL_PIN_COUNT+ANALOG_PIN_COUNT+MCP_PIN_COUNT]; 
uint8_t pinsExtra[DIGITAL_PIN_COUNT+ANALOG_PIN_COUNT+MCP_PIN_COUNT]; 

#define CHANGE_COUNT 4 // This is how many times we repeat the signal that the pin have changed
uint8_t pin_changed[DIGITAL_PIN_COUNT+ANALOG_PIN_COUNT+MCP_PIN_COUNT];


#include "iotypes.h"

#ifdef ROTARY_ENCODER
#include "rotaryEncoder.h"
#endif

#ifdef TM1637
#include "tm1637functions.h"
#endif

#ifdef SERVO
#include "servo.h"
#endif

#ifdef STEPPER
#include "steppermotors.h"
#endif

#ifdef LCD
#include "lcd.h" // // Uses 934 bytes of program memory and 62 bytes of memory
#endif

#ifdef MCP23017x
# include "mcpfunctions.h" // // Uses 964 bytes of program memory and 881 bytes of memory
#endif


int myId = 0; // this will automaticly be set by the chain ping loop
bool cts = true;
bool unconfigured = true;
long loops = 0;
int cyclic = 0;
#include "pinHandle.h"

#ifdef ETHERNET
void setConfig(int nr, int mode, int extra);
#include "network.h" // Uses 5064 bytes of program memory and 253 bytes of memory
#endif




long frametime;
long pingtime;
#define PING_INTERVAL 5000

long refreshtime;


#ifdef TIME_DEBUG
long looptime;
long ditime;
long aitime;
long serialtime;
long chaintime;
long debugtime;
#endif




void readConfig() {
  // Save input/output config in flash memory and read it on boot


  // First read config version string to see if it is compatible
  
}

void setup2() {

  Serial.begin(115200);
  
  pcSerial.begin(115200);
  pcSerial.println("boot");
  
  wdt_reset();
  #ifdef MCP23017x
  Wire.begin();
  #endif
  
  #ifdef ETHERNET
  pinMode(12,OUTPUT);
  # ifdef ETHERNET_RESET
  pcSerial.println("reset ethernet board");
  digitalWrite(12,LOW);
  delay(500);
  wdt_reset();
  digitalWrite(12,HIGH);
  delay(500);
  wdt_reset();
  pcSerial.println("reset ethernet board done");
  # endif
  // disable SD card if one in the slot, this is needed on newer ethernet boards
  pinMode(4,OUTPUT);
  digitalWrite(4,HIGH);
  
  while (setupEthernet() != 1) {
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH); 
    delay(100); 
    digitalWrite(LED_BUILTIN, LOW);
    delay(100);
    digitalWrite(LED_BUILTIN, HIGH); 
    delay(100); 
    digitalWrite(LED_BUILTIN, LOW);
    delay(100);
  }
  wdt_reset();
  
  Serial.println(Ethernet.localIP());
  #endif
  
  #ifdef SERIAL_CHAIN
  chainSerial.begin(115200);
  #endif
  
  wdt_reset();

  // temporary hardcoded setups
  pinsConfig[0] = NOTUSED;
  pinsConfig[1] = NOTUSED;

  
  pinsConfig[DIGITAL_PIN_COUNT+ANALOG_PIN_COUNT+2] = MCP_DI;
  pinsConfig[DIGITAL_PIN_COUNT+2] = DI_INPUT_PULLUP;

  
  wdt_reset();
  setupDigitalPins();
  #ifdef STEPPER
  setupStepper();
  #endif
  wdt_reset();
  #ifdef LCD
  setupLCD();
  #endif

  
  wdt_reset();
  pcSerial.println("Starting version v0.2.0");
}

void setup() {
  wdt_enable(WDTO_2S); //Setup watchdog timeout of 2s.
  setup2();
  wdt_reset();
  
}

void loop() {
  loops++;
  wdt_reset();

                              #ifdef TIME_DEBUG
                              looptime = millis();
                              #endif
  handleDigitalPins();
  
  #ifdef MCP23017x
  readMCPboards();
  #endif
  
  #ifdef ETHERNET
  loopEthernet();
  wdt_reset();
  //pcSerial.println("ethernet done");
  #endif

                              #ifdef TIME_DEBUG
                              ditime = millis() - looptime;
                              aitime = millis();
                              #endif
  
  
                              #ifdef TIME_DEBUG
                              aitime = millis() - aitime;
                              serialtime = millis();
                              #endif
  if (millis()>frametime && cts) {
    frametime = millis() + (1000/FRAMERATE);
    #ifdef TIME_DEBUG
//    Serial.print("loops:");
//    Serial.println(loops);
    #endif
    loops = 0;
    #ifdef ETHERNET
    sendDataEth();
    #else
    sendData();
    #endif
    //cyclicRefresh();
  }
  #ifdef REFRESH_TIME
  if (millis()>refreshtime) {
    refreshtime = millis() + REFRESH_TIME; 
    cyclicRefresh();
  }
  #endif
  if (millis()>pingtime) {
    pingtime = millis() + PING_INTERVAL; 
    ping();
  }
                                #ifdef TIME_DEBUG
                                Serial.flush();
                                serialtime = millis() - serialtime;
                                chaintime = millis();
                                #endif

  #ifdef MASTER
  parsePCSerial();
  #endif
  #ifdef SERIAL_CHAIN
  parseSerial();
  #endif
                                #ifdef TIME_DEBUG
                                Serial.flush();
                                chaintime = millis() - chaintime;
                                #endif
   #ifdef STEPPER
   stepperLoop();
   #endif
   
  #ifdef LCD
  lcdLoop();
  #endif
            
                                #ifdef TIME_DEBUG
                                      if (millis()>debugtime) {
                                        
                                        debugtime = millis() + (5000); 
                                        
                                        looptime = millis() - looptime;
                                      
                                        Serial.print("digital:");
                                        Serial.print(ditime);
                                        //pinMode(50, INPUT_PULLUP);
                                        //pinMode(51, INPUT_PULLUP);
                                        //pinMode(52, INPUT_PULLUP);
                                        Serial.print(" analog:");
                                        Serial.print(aitime);
                                        
                                        Serial.print(" serial:");
                                        Serial.print(serialtime);
                                        
                                        Serial.print(" chain:");
                                        Serial.print(chaintime);
                                        
                                        Serial.print(" loop:");
                                        Serial.print(looptime);

                                        Serial.print(" A0:");
                                        Serial.print(analogRead(A0));
                                        Serial.print(" A1:");
                                        Serial.print(analogRead(A1));
                                        Serial.print(" A2:");
                                        Serial.print(analogRead(A2));
                                        Serial.print(" A3:");
                                        Serial.print(analogRead(A3));
                                        Serial.print(" A4:");
                                        Serial.print(analogRead(A4)); 
                                        Serial.print(" A5:");
                                        Serial.println(analogRead(A5));   
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
  bool changes = false;
  for (int i = 0; i<DIGITAL_PIN_COUNT+ANALOG_PIN_COUNT+MCP_PIN_COUNT; i++) {
    if (pin_changed[i]) {
      changes = true;
      break;
    }
  }
  if (changes != true) {
    return;
  }
  dataSerial.print("{99;");
  #ifdef MASTER
  dataSerial.print(MASTER_ID);
  dataSerial.print(";");
  #endif
  dataSerial.print(myId);
  dataSerial.print(";");
  for (int i = 0; i<DIGITAL_PIN_COUNT; i++) {
    if (pin_changed[i]) {
      pin_changed[i]--;
      dataSerial.print("D");
      dataSerial.print(i);
      dataSerial.print(" ");
      dataSerial.print(pinsData[i]);
      dataSerial.print(",");
    }
  }
  for (int i = DIGITAL_PIN_COUNT; i<DIGITAL_PIN_COUNT+ANALOG_PIN_COUNT; i++) {
    if (pin_changed[i]) {
      pin_changed[i]--;
      dataSerial.print("A");
      dataSerial.print(i-DIGITAL_PIN_COUNT);
      dataSerial.print(" ");
      dataSerial.print(pinsData[i]);
      dataSerial.print(",");
    }
  }
  for (int i = DIGITAL_PIN_COUNT+ANALOG_PIN_COUNT; i<DIGITAL_PIN_COUNT+ANALOG_PIN_COUNT+MCP_PIN_COUNT; i++) {
    if (pin_changed[i]) {
      pin_changed[i]--;
      dataSerial.print("M");
      dataSerial.print(i-DIGITAL_PIN_COUNT-ANALOG_PIN_COUNT);
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
        doSomething(chainSerial);
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

void waitForData(HardwareSerial& inSerial, int nr) {
  int counter = 0;
  while (inSerial.available() < nr) {
    // nop
    asm("nop;");
    counter++;
    if (counter > 1000) {
      pcSerial.println("waiting...");
      counter = 0;
    }
  }
}
#ifdef MASTER
void parsePCSerial() {
  // Check if the serial data is for me
  // if not send it to next device in chain
  while (pcSerial.available() > 0) {
    if (pcSerial.read() == '{') {
      waitForData(pcSerial, 3);
      int masterid = pcSerial.parseInt();
      waitForData(pcSerial, 3);
      char data = pcSerial.read(); // read the ; character
      waitForData(pcSerial, 3);
      int slaveid = pcSerial.parseInt();

      pcSerial.print("got master");
      pcSerial.print(masterid);
      
      pcSerial.print(" slave");
      pcSerial.print(slaveid);
      
      if (masterid == MASTER_ID) {
        pcSerial.println("data for me or slaves");
        // data for me or my slaves
      
        if ( slaveid == 0 ) {
          // data is for me
          doSomething(pcSerial);
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
        pcSerial.println("else if");
        // relay to other masters???
        
      }
      else {
        pcSerial.println("broadcast");
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

void doSomething(HardwareSerial& inSerial) {
  // message for me
  char data;
  
  pcSerial.print("message for me: ");
  waitForData(inSerial, 1);
  data = inSerial.read(); // read the ; char
  waitForData(inSerial, 3);
  int messageType = inSerial.parseInt();
  if (messageType == 0) {
    // set pin message
    handleSetPinMessage(inSerial);
  }else if (messageType == 1) {
    // set config
    handleConfigMessage(inSerial);
  }else if (messageType == 2) {
    // set config and save
    handleConfigMessage(inSerial);
    saveConfig();
  }

}

int readPinNr(HardwareSerial& inSerial, char sep) {
  char da[5];
  int counter = 0;
  char data;
  waitForData(inSerial, 1);
  data = inSerial.read();
  if (data == '}') {
    return -1;
  }
  while (data != sep) {
    // read data until we find ','
    da[counter] = data;
    counter++;
    waitForData(inSerial, 1);
    data = inSerial.read();
    if (counter == 4) {
      break;
    }
  }
  da[counter] = '\0'; // replace , with end of string char
  int pinNr = 0;
  if (da[0] == 'D') {
    // all good
    da[0] = ' ';
  }else if (da[0] == 'A') {
    // all good add offset for analog pin
    pinNr = pinNr + DIGITAL_PIN_COUNT;
    da[0] = ' ';
  } else if (da[0] == 'M') {
    // all good add offset for mcp pin
    pinNr = pinNr + DIGITAL_PIN_COUNT + ANALOG_PIN_COUNT;
    da[0] = ' ';
  } else {
    // not good
    return -1;
  }
  int x = atoi(da);
  pinNr = pinNr + x;
  return pinNr;
}
void handleSetPinMessage(HardwareSerial& inSerial) {
//  pcSerial.print("handleSetPinMessage: ");
  char data;
  waitForData(inSerial, 2);
  data = inSerial.read(); // read ; char
  while( data == ';') {
    int pinNr = readPinNr(inSerial, '=');
    if (pinNr == -1) {
      break;
    }
    int value = inSerial.parseInt();
    
//        pcSerial.print("found pinnr: ");
//        pcSerial.print(pinNr);
//        pcSerial.print("value: ");
//        pcSerial.println(value);
    setValue(pinNr, value);
    waitForData(inSerial, 1);
    data = inSerial.read();
//    pcSerial.println(data);
  }
  
//  pcSerial.println("end handleSet");
}

void handleConfigMessage(HardwareSerial& inSerial) {
  // parse the message {13;0;2;D3,1,0;A2,4,3;}
  // from this position       ^_____________
  
//  pcSerial.print("handleConfigMessage: ");
  char data;
  waitForData(inSerial, 2);
  data = inSerial.read(); // read ; char
  while( data == ';') {
    int pinNr = readPinNr(inSerial, ',');
    if (pinNr == -1) {
      break;
    }
    int pinmode = inSerial.parseInt();
    waitForData(inSerial, 1);
    data = inSerial.read();
    int pinExtra = inSerial.parseInt();
//        pcSerial.print("found pinnr: ");
//        pcSerial.print(pinNr);
//        pcSerial.print("mode: ");
//        pcSerial.print(pinmode);
//        pcSerial.print("extra: ");
//        pcSerial.println(pinExtra);
    setConfig(pinNr, pinmode, pinExtra);
    waitForData(inSerial, 2);
    data = inSerial.read();
//    pcSerial.println(data);
  }
  
  setupAllPins();
//  pcSerial.println("end handleConfig");
}

void setConfig(int nr, int mode, int extra) {
  pinsConfig[nr] = mode;
  pinsExtra[nr] = extra; 
  pcSerial.print("setConfig pinnr: ");
  pcSerial.print(nr);
  pcSerial.print("mode: ");
  pcSerial.print(mode);
  pcSerial.print("extra: ");
  pcSerial.println(extra);
}
void saveConfig() {
  // TODO
}
void relayToPC() {
  #ifdef ETHERNET 
  relayToPCEth(); // in network.h
  #else
  // message to PC from slave via serial
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
  #endif
}

void cyclicRefresh() {

  while(pinsConfig[cyclic]<1) {
    cyclic++;
    if (cyclic > DIGITAL_PIN_COUNT+ANALOG_PIN_COUNT+MCP_PIN_COUNT) {
      cyclic =0;
      break;
    }
  }
  
  
  if (pinsConfig[cyclic]!=0) {

    pin_changed[cyclic]++;
    
  } 
  cyclic++;
  
}
