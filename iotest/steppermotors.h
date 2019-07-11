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
  int rotating; // 1 = Like a compass we go the shortest way to reach target. 0 = Linear like a speedometer
  int currentSteps;
  int pin; // this gets the same value as the first pin used for the stepper as index for our data transfer
  int rpm;
} myStep;


// ############## here you have to add each stepper and the pins connected to it ################
myStep stepp[] = {
  //{Stepper(2048, 8, 10, 9, 11), 2048, 1, 0, 8, 15},
  {Stepper(2048, 2, 4, 3, 5), 2048, 1, 0, 2, 10}
};

void setupStepper() {

  for (int i = 0; i < NR_STEPPERS; i++) {
    stepp[i].s.setSpeed(stepp[i].rpm);
  }

}

int degToSteps(int stepsPerRev, int targetDeg) {
  long s = stepsPerRev;
  long t = targetDeg;
  long temp = s*t;
  temp = temp / ONE_TURN;
  int out = temp;
  return out;
}


void stepperLoop() {
  
  for (int i = 0; i < NR_STEPPERS; i++) {
    int target = degToSteps(stepp[i].stepsPerRev, pinsData[stepp[i].pin]);
    if (target != stepp[i].currentSteps) {
      int s = target - stepp[i].currentSteps;
      if (abs(s) > MAXSTEPSPERLOOP) {
        // only do some stepping to avoid locking the loop
        if (s>0) {
          s = MAXSTEPSPERLOOP;
        } else {
          s = 0 - MAXSTEPSPERLOOP;
        }
      }
      stepp[i].currentSteps = stepp[i].currentSteps + s;
      stepp[i].s.step(s);
    }
  }

}






#endif
