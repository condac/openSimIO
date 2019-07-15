// If your stepper motor can only rotate in one direction swap th pin2 and pin3 to the stepper driver. This is common for cheap steppermotors labeled 28BYJ-48

#ifdef STEPPER

#include <Stepper.h>


#define MAXSTEPSPERLOOP 10

#define ONE_TURN 3600

// ############## You must enter how many steppers you have setup here ##############
#define NR_STEPPERS 1

typedef struct  {
  Stepper s;
  int stepsPerRev;
  bool rotating; // 1 = Like a compass we go the shortest way to reach target. 0 = Linear like a speedometer
  bool inverted; // change rotating direction
  int currentSteps;
  int pin; // this gets the same value as the first pin used for the stepper as index for our data transfer
  int rpm;
  int homePin;
} myStep;


// ############## here you have to add each stepper and the pins connected to it ################
myStep stepp[] = {
  //{Stepper(2048, 8, 10, 9, 11), 2048, true, false, 0, 8, 15,7},
  {Stepper(2048, 2, 4, 3, 5), 2048, true, true, 0, 2, 10,7}
};


int degToSteps(int stepsPerRev, int targetDeg) {
  long s = stepsPerRev;
  long t = targetDeg;
  long temp = s*t;
  temp = temp / ONE_TURN;
  int out = temp;
  return out;
}

void homeStepper(int i) {
  pinMode(stepp[i].homePin, INPUT_PULLUP);
  if (!digitalRead(stepp[i].homePin)) {
    // homePin is triggered, move the stepper 1/4 turn so we get the sensor untriggered, then move back untill we are home again
    stepp[i].s.step(stepp[i].stepsPerRev/4);
  }
  int st = 0;
  while ( digitalRead(stepp[i].homePin) ) {
    stepp[i].s.step(-1);
    st++;
    if (st > stepp[i].stepsPerRev) {
      // did not find home
      break;
    }
  }
  // now we are home
  stepp[i].currentSteps = 0;
}

void setupStepper() {

  for (int i = 0; i < NR_STEPPERS; i++) {
    stepp[i].s.setSpeed(stepp[i].rpm);
    if (stepp[i].homePin != 0) {
      homeStepper(i);
    }
    
  }

}




void stepperLoop() {
  
  for (int i = 0; i < NR_STEPPERS; i++) {
    int target = degToSteps(stepp[i].stepsPerRev, pinsData[stepp[i].pin]);
    if (target != stepp[i].currentSteps) {
      int s = target - stepp[i].currentSteps;
      if (stepp[i].rotating) {
        
        // Find out if we can do a short cut and rotate in wrong direction
        if (abs(s) > stepp[i].stepsPerRev/2) {
           if (s>=0) {
            s = stepp[i].stepsPerRev - s;
            s = -s;
          } else {
            s = stepp[i].stepsPerRev + s;
            
          }

        }
      }
      if (abs(s) > MAXSTEPSPERLOOP) {
        // only do some stepping to avoid locking the loop
        if (s>0) {
          s = MAXSTEPSPERLOOP;
        } else {
          s = 0 - MAXSTEPSPERLOOP;
        }
      }
      stepp[i].currentSteps = stepp[i].currentSteps + s;
      if (stepp[i].rotating) {
        if (stepp[i].currentSteps > stepp[i].stepsPerRev) {
          stepp[i].currentSteps = stepp[i].currentSteps -stepp[i].stepsPerRev;
        } else if (stepp[i].currentSteps < 0) {
          stepp[i].currentSteps = stepp[i].currentSteps + stepp[i].stepsPerRev;
        }
      }
      if (stepp[i].inverted) {
        stepp[i].s.step(-s);
      } else {
        stepp[i].s.step(s);
      }
      
    }
  }

}






#endif
