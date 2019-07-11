#include "openSimIO.h"

int getTypeFromString(char* string) {

  //#define DI_INPUT_PULLUP 1 // Will return 1 when button is pressed, going low to ground
  if (strcmp(string, "DI_INPUT_PULLUP") == 0) { return 1;}
  //#define DI_INPUT_FLOATING 2
  if (strcmp(string, "DI_INPUT_FLOATING") == 0) { return 2;}
  //#define DI_INPUT_STEP 3 // Will step increase a value while holding the button presed
  if (strcmp(string, "DI_INPUT_STEP") == 0) { return 3;}
  //#define DO_BOOL 129 // digital out 1 or 0
  if (strcmp(string, "DO_BOOL") == 0) { return 129;}
  //#define AI_RAW 4 // Use only with main flight control, this will most likely update every frame
  if (strcmp(string, "AI_RAW") == 0) { return 4;}
  //#define AI_FILTER 5 // Use this, Noize and deadband filtered to avoid lot of communication spam.
  if (strcmp(string, "AI_FILTER") == 0) { return 5;}
  //#define AI_OVERSAMPLE 6 // TODO // Read the value multiple times to fake higher resolution, for main flight controls that have limited use of potensoimeter
  if (strcmp(string, "AI_OVERSAMPLE") == 0) { return 6;}
  //#define AO_PWM 130 // TODO // Only on PWM pins
  if (strcmp(string, "AO_PWM") == 0) { return 130;}
  //#define AO_DAC 131 // TODO // Only on DUE boards
  if (strcmp(string, "AO_DAC") == 0) { return 131;}
  //#define AO_SERVO 132 // TODO
  if (strcmp(string, "AO_SERVO") == 0) { return 132;}
  //#define DI_ROTARY_ENCODER_TYPE1 10
  if (strcmp(string, "DI_ROTARY_ENCODER_TYPE1") == 0) { return 10;}
  //#define DO_HIGH 133 // Use with care, do not load more than 20mA on pin!
  if (strcmp(string, "DO_HIGH") == 0) { return 133;}
  //#define DO_LOW 134 // Use with care, do not load more than 20mA on pin!
  if (strcmp(string, "DO_LOW") == 0) { return 134;}
  //#define USED 13 // Used by functions that need extra pins, not sure if we need this or just use 0
  if (strcmp(string, "USED") == 0) { return 13;}
  //#define DI_3WAY 14 // TODO 3-way switch
  if (strcmp(string, "DI_3WAY") == 0) { return 14;}
  //#define DI_3WAY_2 15 // 3 way switch with special function, uses 4 wires
  if (strcmp(string, "DI_3WAY_2") == 0) { return 15;}
  //#define DO_TM1637_DEC 135 // Display decimal number
  if (strcmp(string, "DO_TM1637_DEC") == 0) { return 135;}
  //#define DI_4X4 16 // 4x4 key matrix
  if (strcmp(string, "DI_4X4") == 0) { return 16;}
  //#define AO_STEPPER 136 // Stepper motor
  if (strcmp(string, "AO_STEPPER") == 0) { return 136;}
  return 0;
}
