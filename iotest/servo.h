#include <Servo.h>

#define NR_SERVOS DIGITAL_PIN_COUNT // 10 servos takes 27 bytes memory
Servo myservos[NR_SERVOS];
void setupServo(int pin) {
  
  myservos[pin].attach(pin); 
}
void setServo(int pin, int val) {
//  pcSerial.println("Servo");
  //Servo myservo;
  //myservo.attach(pin); 
  myservos[pin].write(val);
  //delay(15); 
}
