#include <Wire.h>
#include <MCP23017.h> // From official repo created by Bertrand Lemastle https://github.com/blemasle/arduino-mcp23017

#define MCP23017_ADDR 0x20
#define MCP_INPUT 0
#define MCP_OUTPUT 1

MCP23017 *mcp[MCP23017_MAX_BOARDS];

bool mcp_board_setup[MCP23017_MAX_BOARDS];
bool mcp_board_mode[MCP23017_MAX_BOARDS][2];

uint8_t mcp_board_outpins[MCP23017_MAX_BOARDS][2];
//bool mcp_board_pin[MCP23017_MAX_BOARDS][16];


void setupMCPboard(int nr, int mode, int pin_onboard) {
  
  int addr = 0x20;
  addr = addr+nr;
//  Serial.print("setupMCPboard:");
//  Serial.print(nr);
//  Serial.print(" addr");
//  Serial.print(addr);
//  Serial.print(" mode");
//  Serial.println(mode);
  mcp[nr] =  new MCP23017(addr);
  mcp[nr]->init();

  
  mcp_board_setup[nr] = 1;
}

void setupMCPpin(int pin) {
//    Serial.print("setupMCPpin:");

//  Serial.println(pin);
  int mcp_pin = pin - (DIGITAL_PIN_COUNT+ANALOG_PIN_COUNT);
  int board = mcp_pin/16;
  int pin_onboard = mcp_pin-(board*16);
  int mode = pinsConfig[pin];
//  Serial.print("mcp_pin:");
//  Serial.print(mcp_pin);
//  Serial.print(" board:");
//  Serial.print(board);
//  Serial.print("pin_onboard:");
//  Serial.println(pin_onboard);
  if (!mcp_board_setup[board]) {
    
    setupMCPboard(board, mode, pin_onboard);
    
  }
  
  if (pin_onboard>=0 and pin_onboard<=7) {
    if (mode == MCP_DI) {
      //Serial.println("set A as input");
      mcp[board]->portMode(MCP23017Port::A, 0b11111111); //Port A as input
      mcp[board]->writeRegister(MCP23017Register::GPPU_A, 0xFF);
      mcp_board_mode[board][0] = MCP_INPUT;
    }
    else {
      //Serial.println("set A as output");
      mcp[board]->portMode(MCP23017Port::A, 0);          //Port A as output
      mcp_board_mode[board][0] = MCP_OUTPUT;
    }
    mcp[board]->writeRegister(MCP23017Register::GPIO_A, 0x00);  //Reset port A 
  }
  
  if (pin_onboard>=8 and pin_onboard<=15) {
    if (mode == MCP_DI) {
      //Serial.println("set B as input");
      mcp[board]->portMode(MCP23017Port::B, 0b11111111); //Port B as input
      mcp[board]->writeRegister(MCP23017Register::GPPU_B, 0xFF);
      mcp_board_mode[board][1] = MCP_INPUT;
    }
    else {
      //Serial.println("set B as output");
      mcp[board]->portMode(MCP23017Port::B, 0);           //Port B as output
      mcp_board_mode[board][1] = MCP_OUTPUT;
    }
    mcp[board]->writeRegister(MCP23017Register::GPIO_B, 0x00);  //Reset port B
  }
}

void readMCPboards() {

  //Serial.println("readMCPboards");
  for (int i = 0; i<MCP23017_MAX_BOARDS; i++) {
    if (mcp_board_setup[i]) {
      if (mcp_board_mode[i][0] == MCP_INPUT) {
        uint8_t currentA;
  
        currentA = mcp[i]->readPort(MCP23017Port::A);
//        Serial.print("readPortA:");
//        Serial.println(currentA,BIN);
        for (int x = 0; x<8; x++) {
          int pin = (DIGITAL_PIN_COUNT+ANALOG_PIN_COUNT) + (16*i)+x;
          bool currentState = bitRead(currentA,x);
          if (currentState != pinsData[pin]) {
            pin_changed[pin] = CHANGE_COUNT;
            pinsData[pin] = currentState;
          }
        }
      }
      if (mcp_board_mode[i][1] == MCP_INPUT) {
        
        uint8_t currentB;
        currentB = mcp[i]->readPort(MCP23017Port::B);
        for (int x = 0; x<8; x++) {
          int pin = (DIGITAL_PIN_COUNT+ANALOG_PIN_COUNT) + (16*i)+x+8;
          bool currentState = bitRead(currentB,x);
          if (currentState != pinsData[pin]) {
            pin_changed[pin] = CHANGE_COUNT;
            pinsData[pin] = currentState;
          }
        }
      }
    }
  }
}

void setMCPpin(int pin, int val) {
  
  
  //return; // TODO



  int mcp_pin = pin - (DIGITAL_PIN_COUNT+ANALOG_PIN_COUNT);
  int board = mcp_pin/16;
  int pin_onboard = mcp_pin-(board*16);
  int port = pin_onboard/8;
//  Serial.print("setMCPpin");
//  Serial.print(pin);
//  Serial.print(" board");
//  Serial.print(board);
//  Serial.print(" port");
//  Serial.print(board);
//  Serial.print(" bit");
//  Serial.println(pin_onboard-port*8);

  if (val) {
    bitSet(mcp_board_outpins[board][port], pin_onboard-port*8);
  }
  else {
    bitClear(mcp_board_outpins[board][port], pin_onboard-port*8);
  }
}

void writeMCPBoards() {



  
  //return; // TODO



  
  for (int i = 0; i<MCP23017_MAX_BOARDS; i++) {
    if (mcp_board_setup[i]) {
      if (mcp_board_mode[i][0] == MCP_OUTPUT) {
        uint8_t currentA;
        currentA = mcp_board_outpins[i][0];
        mcp[i]->writePort(MCP23017Port::A, currentA);
      }
      if (mcp_board_mode[i][1] == MCP_OUTPUT) {
        
        uint8_t currentB;
        currentB = mcp_board_outpins[i][1];
        mcp[i]->writePort(MCP23017Port::B, currentB);
        //Serial.print("writeboard b ");
        //Serial.println(currentB);
        
      }
    }
  }
}
