#include "openSimIO.h"
#include <stdio.h>
#include <string.h>

typedef struct  {

	int master;
  int slave;
  int ioMode;
  XPLMDataRef dataRef;
  float pinMin;
  float pinMax;
  float xplaneMin;
  float xplaneMax;
  float center;
  int reverse;
  int pinNr;
  char pinNameString[10];
	int output;
	int prevValue;
	float prevValueF;

} pin_struct;

FILE *configFile;

int nrOfLines = 0;
int nrOfPins = 0;

pin_struct *pins;

pin_struct* lineToStruct( char* line) {

  // convert config line to struct and return it as pointer
  pin_struct *newPin = malloc(sizeof (*newPin));
  if(line[0] == '#') {
    return NULL;
  }

  //char pinNameString[10];
  char ioTypeString[32];
  char dataRefString[512];

  int conversionCount = sscanf(line, "%d.%d.%4[^;];%d;%d;%f;%f;%f;%512[^;];%f;%f;", &newPin->master,
                                                                                         &newPin->slave,
                                                                                         &newPin->pinNameString,
                                                                                         &newPin->ioMode,
                                                                                         &newPin->reverse,
                                                                                         &newPin->center,
                                                                                         &newPin->pinMin,
                                                                                         &newPin->pinMax,
                                                                                         dataRefString,
                                                                                         &newPin->xplaneMin,
                                                                                         &newPin->xplaneMax );
  //int conversionCount = sscanf(line, "%d.%d.%4[^;];%31[^;];%d;%f;%f;%f;", &newPin->master, &newPin->slave, pinNameString, ioTypeString, &newPin->reverse, &newPin->center, &newPin->pinMin, &newPin->pinMax);
  if(conversionCount != 11) {
      display("Error! converting config line %s", line);
      return NULL;
  } else {
    display("master %d slave %d %s %d %d %f %f %f %s ", newPin->master, newPin->slave, &newPin->pinNameString, &newPin->ioMode, newPin->reverse, newPin->center, newPin->pinMin, newPin->pinMax, dataRefString);

		if (newPin->ioMode > 127) {
			newPin->output = 1;
		}
		else {
			newPin->output = 0;
		}
		newPin->dataRef = XPLMFindDataRef(dataRefString);

    if (newPin->dataRef == NULL)	{
      display("dataRef invalid %s", dataRefString);
      return NULL;
    }
    return newPin;

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

float mapValue(float value, float min, float max, float center, float outMin, float outMax, int reverse) {
  float out = ((float)value/512.0) - 1.0;

  if (value>max) {
    value = max;
  }
  if (value<min) {
    value = min;
  }

  if (value>center) {
    value = value - center;
    float scale = outMax / (max-center) ;
    out = value * scale;
  } else {
    value = value - center;
    float scale = outMin / (min - center);
    if (scale < 0) {
      scale = scale * (-1.0);
    }
    out = value * scale;
  }
  if (reverse == 1) {
    out = out*-1.0;
  }

  return out;
}
void setAnalogData(int master, int slave, char* pinName, int value) {
  display("setAnalogData %s %d", pinName, value);
  for ( int i=0; i<nrOfPins;i++) {

    if (strcmp(pinName, pins[i].pinNameString) == 0) {
      display("found pinname %s", pinName);

      int type = XPLMGetDataRefTypes(pins[i].dataRef);

      if (type == xplmType_Int) {
        int setValue = (int) value;

        XPLMSetDatai(pins[i].dataRef,setValue);
      } else if (type == xplmType_Float) {
        float setValue = mapValue(value, pins[i].pinMin, pins[i].pinMax, pins[i].center, pins[i].xplaneMin, pins[i].xplaneMax, pins[i].reverse);
        display("setAnalogData setting float %s %f", pinName, setValue);
        XPLMSetDataf(pins[i].dataRef,setValue);
      } else if (type == xplmType_Double) {
        double setValue = (double) mapValue(value, pins[i].pinMin, pins[i].pinMax, pins[i].center, pins[i].xplaneMin, pins[i].xplaneMax, pins[i].reverse);
        //display("setAnalogData setting double %s %f", pinName, setValue);
        XPLMSetDatad(pins[i].dataRef,setValue);
      }
    }
  }

}

void setDigitalData(int master, int slave, char* pinName, int value) {
  display("setDigitalData %s %d", pinName, value);
  for ( int i=0; i<nrOfPins;i++) {

    if (strcmp(pinName, pins[i].pinNameString) == 0) {
      display("found pinname %s", pinName);

      int type = XPLMGetDataRefTypes(pins[i].dataRef);

      if (type == xplmType_Int) {
        int setValue = (int) value;
        display("setDigitalData setting int %s %d", pinName, setValue);
        XPLMSetDatai(pins[i].dataRef,setValue);
      } else if (type == xplmType_Float) {
        float setValue = mapValue(value, pins[i].pinMin, pins[i].pinMax, pins[i].center, pins[i].xplaneMin, pins[i].xplaneMax, pins[i].reverse);
        //display("setAnalogData setting float %s %f", pinName, setValue);
        XPLMSetDataf(pins[i].dataRef,setValue);
      } else if (type == xplmType_Double) {
        double setValue = (double) mapValue(value, pins[i].pinMin, pins[i].pinMax, pins[i].center, pins[i].xplaneMin, pins[i].xplaneMax, pins[i].reverse);
        //display("setAnalogData setting double %s %f", pinName, setValue);
        XPLMSetDatad(pins[i].dataRef,setValue);
      }
    }
  }

}

void setDigitalPin(int cport_nr,int pin, int value ) {
	char out[512];
	int count = 0;
 // send digital data to arduino
	if (pins[pin].output == 1) {
		if (pins[pin].prevValue != value) {

			int len = sprintf(out, "{%d;%d;0;%s=%d;}", pins[pin].master, pins[pin].slave, pins[pin].pinNameString, value);
			display("write serial:%s", out);
			RS232_SendBuf(cport_nr, out, len+1);
			pins[pin].prevValue = value;
		}
	}
}
void sendDataToArduino(int cport_nr) {
	char out[512];
	int count = 0;

	for (int i=0;i<nrOfPins;i++) {
		if (pins[i].output == 1) {
			int type = XPLMGetDataRefTypes(pins[i].dataRef);

      if (type == xplmType_Int) {

        setDigitalPin(cport_nr,i,XPLMGetDatai(pins[i].dataRef) );
      } else if (type == xplmType_Float) {
        //float setValue = mapValue(value, pins[i].pinMin, pins[i].pinMax, pins[i].center, pins[i].xplaneMin, pins[i].xplaneMax, pins[i].reverse);
        //display("setAnalogData setting float %s %f", pinName, setValue);
        //XPLMSetDataf(pins[i].dataRef,setValue);
      } else if (type == xplmType_Double) {
        //double setValue = (double) mapValue(value, pins[i].pinMin, pins[i].pinMax, pins[i].center, pins[i].xplaneMin, pins[i].xplaneMax, pins[i].reverse);
        //display("setAnalogData setting double %s %f", pinName, setValue);
        //XPLMSetDatad(pins[i].dataRef,setValue);
      }
		}
	}
}
