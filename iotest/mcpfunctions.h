#include <Wire.h>
#include <MCP23017.h> // From official repo created by Bertrand Lemastle https://github.com/blemasle/arduino-mcp23017

#define MCP23017_ADDR 0x20
#define MCP_INPUT 0
#define MCP_OUTPUT 1

MCP23017 *mcp[MCP23017_MAX_BOARDS];

bool mcp_board_setup[MCP23017_MAX_BOARDS];
bool mcp_board_mode[MCP23017_MAX_BOARDS][2];
//bool mcp_board_pin[MCP23017_MAX_BOARDS][16];


void setupMCPboard(int nr, int mode) {
  Serial.print("setupMCPboard:");
  Serial.print(nr);
  Serial.print(" ");
  Serial.println(mode);
  int addr = 0x20;
  addr = addr+nr;
  mcp[nr] =  new MCP23017(addr);
  mcp[nr]->init();
  if (mode == MCP_DI) {
    mcp[nr]->portMode(MCP23017Port::A, 0b11111111); //Port A as input
    mcp[nr]->portMode(MCP23017Port::B, 0b11111111); //Port B as input
    mcp[nr]->writeRegister(MCP23017Register::GPPU_A, 0xFF);
    mcp[nr]->writeRegister(MCP23017Register::GPPU_B, 0xFF);
    mcp_board_mode[nr][0] = MCP_INPUT;
    mcp_board_mode[nr][1] = MCP_INPUT;
  }
  else {
    mcp[nr]->portMode(MCP23017Port::A, 0);          //Port A as output
    mcp[nr]->portMode(MCP23017Port::B, 0);           //Port B as output
    mcp_board_mode[nr][0] = MCP_OUTPUT;
    mcp_board_mode[nr][1] = MCP_OUTPUT;
  }
  

  mcp[nr]->writeRegister(MCP23017Register::GPIO_A, 0x00);  //Reset port A 
  mcp[nr]->writeRegister(MCP23017Register::GPIO_B, 0x00);  //Reset port B
  mcp_board_setup[nr] = 1;
}

void setupMCPpin(int pin) {
    Serial.print("setupMCPpin:");

  Serial.println(pin);
  int mcp_pin = pin - (DIGITAL_PIN_COUNT+ANALOG_PIN_COUNT);
  int board = mcp_pin/16;
  if (!mcp_board_setup[board]) {
    int mode = pinsConfig[pin];
    setupMCPboard(board, mode);
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

}
