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

// Refresh interval. This will read and resend all inputs to fix issues where switches are in one position and xplane is reloaded and think it is another position
// It will trigger a resend every ms defined by REFRESH_TIME but only for one value at the time. So if you have 20 inputs configured setting REFRESH_TIME to 1000 will take 20s for all values to have be resent.
// If you want to be able to control switches from within the game and not have them overwritten uncomment this.
#define REFRESH_TIME 1000 // refresh interval, comment out to disable

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

// Use TM1637 drivers for 7 segment displays
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
