
// Warning! this code is blocking the main loop with long delays while someone is turning the rotary.
// Interupts can only be placed on a certain number of pins, this code is interupt free to the cost of blocking. 
// While the board is under light load this works totaly fine, and the shortest pulse is catched 3 times in the loop. 

//int rotationArray[DIGITAL_PIN_COUNT]; // TODO find a more memory saving way to save rotations, on mega this eats 108 bytes of memory


bool getRotationType1(int pinIn1, int pinIn2) {
    // Type1 in this project is a rotary encoder that makes one pulse for one turn. it lower one pin before the other
    // But it also rise back one pin before the other.
    // So there is 2 ways to read the rotation, at the beginning or at the end. 
    // We can miss the start because the program loop does other things, but if we are lucky we can catch the beginning
    // if we did not catch the beginning we wait for the end and make a conclution

    /*  Typical pulse. 5-30ms long ( vary much)
     * 
     * pin1  ------------__________------
     * pin2  -----________________-------
     */
    
  bool pin1Start = digitalRead(pinIn1);
  bool pin2Start = digitalRead(pinIn2);
  
  // pins are high (true) when noone is turning the device
  if (!pin1Start || !pin2Start) {
    // someone is turning the device. Now we want to findout if we are at the beginning or end of pulses
    if (!pin1Start && !pin2Start) {
      // both pins are triggered, this means we are in the middle of a cycle and we missed the beginning
      
    }
    else {
      // Only one of the pins are triggered, this means we are at the beginning OR at the end cycle
      bool running = true; // we might need a loop in a loop so we have a variable to stop the outer loop
      bool pin1 = digitalRead(pinIn1);
      bool pin2 = digitalRead(pinIn2);
      while (running) {
        if (pin1 && pin2) {
          // if both are high this means the trigger is over, in that case we started in the end of the cycle
          if (!pin1Start) {
            pinsData[pinIn1] ++;
            running = false;
          }else {
            pinsData[pinIn1] --;
            running = false;
          }
          
        }
        if (!pin1 && !pin2) {
          // Now both pins are low, this means that we have started with one pin low and now have two pins low. 
          // Now it is easy to see in what direction we went
          if (pin1Start) {
            pinsData[pinIn1] ++;
            running = false;
          }else {
            pinsData[pinIn1] --;
            running = false;
          }

          // 
        }
        
        pin1 = digitalRead(pinIn1);
        pin2 = digitalRead(pinIn2);
        
      }
    }
    //pcSerial.println(rotationArray[pinIn1]);
   pin_changed[pinIn1] = true;
   return true;
  }
  else {
    return false; 
  }

}
