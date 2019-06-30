#include "openSimIO.h"
#include "iotypes.h"

typedef struct  {

	int master;
  int slave;
  int iotype;
  XPLMDataRef dataRef;
  float pinMin;
  float pinMax;
  float xplaneMin;
  float xplaneMax;
  float center;
  int reverse;
  int pinNr;

} pin_struct;

FILE *configFile;

int nrOfLines = 0;
int nrOfPins = 0;

pin_struct *pins;

pin_struct* lineToStruct( char* line) {
  pin_struct *newPin = malloc(sizeof (*newPin));
  if(line[0] == '#') {
    return NULL;
  }

  char pinNameString[10];
  char ioTypeString[32];
  char dataRefString[512];

  //int conversionCount = sscanf(line, "%d.%d.%4[^;];%31[^;];%d;%f;%f;%f;%512[^;];%f;%f;", &newPin->master, &newPin->slave, pinNameString, ioTypeString, &newPin->reverse, &newPin->center, &newPin->pinMin, &newPin->pinMax, dataRefString, &newPin->xplaneMin, &newPin->xplaneMax );
  int conversionCount = sscanf(line, "%d.%d.%4[^;];%31[^;];%d;%f;", &newPin->master, &newPin->slave, pinNameString, ioTypeString, &newPin->reverse, &newPin->center);
  if(conversionCount != 6) {
      display("Error! converting %d config line %s", conversionCount, line);
      return NULL;
  } else {
    display("master %d slave %d %s %s %d %f ", newPin->master, newPin->slave, pinNameString, ioTypeString, newPin->reverse, newPin->center);
  }

  return NULL;
}

void readConfig() {
  if ((configFile = fopen("Resources/plugins/openSimIO/config.txt","r")) == NULL){
       display("Error! opening configfile");

  } else {

    char * line = NULL;
    size_t len = 0;
    ssize_t read;
    while ((read = getline(&line, &len, configFile)) != -1) {
      nrOfLines++;
    }
    fclose(configFile);

    display("lines in config file %d", nrOfLines);

    pins = malloc(nrOfLines * sizeof(pin_struct));

    if ((configFile = fopen("Resources/plugins/openSimIO/config.txt","r")) == NULL){
         display("Error! opening configfile");

    } else {
      char * line = NULL;
      size_t len = 0;
      ssize_t read;
      while ((read = getline(&line, &len, configFile)) != -1) {
        printf("Retrieved line of length %zu:\n", read);
        printf("%s", line);
        display("%s", line);
        pin_struct* newPin = lineToStruct(line);
        if (newPin != NULL) {
          memcpy(pins+nrOfPins, newPin, sizeof(pin_struct));
          nrOfPins++;
        }
      }
    }
  }

}


void setAnalogPin() {


}


void setDigitalPin() {


}
