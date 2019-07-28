// ############
// User configurations
// ############


// Uncomment the master or slave define, never both!
#define MASTER
//#define SLAVE

#define MASTER_ID 1 // Change if you have more than one master board in your system

#define PC_ID 99 // do not touch

// Enable or disable the serial chain function. 
//#define SERIAL_CHAIN


#ifdef SLAVE
#define FRAMERATE 15
#else
#define FRAMERATE 60
#endif

// ############
// Plugins
// ############  IMPORTANT!!!! #########
// To save program memory we only include functions that you know you will use. 
// Uncomment the define statement for each plugin you need in your hardware

// Use rotary encoders
#define ROTARY_ENCODER


// Use ethernet shield
//#define ETHERNET
#ifdef ETHERNET
#include <Ethernet.h>
byte mac[] = {
  0x5A, 0xAB, 0x10, 0x50, 0x7E, 0x57
};
IPAddress ip(192, 168, 0, 105);
#endif

// Use TM1637 drivers fopr 7 segment displays
#define TM1637

// Servo
#define SERVO

// Stepper motors
// NOTE! You need to configure steppermotors in steppermotors.h
// There are just to many options to do this through the plugin
//#define STEPPER



// ############
// End of plugins
// ############
