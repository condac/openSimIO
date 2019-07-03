#include "openSimIO.h"
#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include "iotypes.h"
#include "udp.h"
#include "rs232.h"

typedef struct  {

	int master;
  int slave;
  int ioMode;
  XPLMDataRef dataRef;
  int dataRefIndex;
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
	int extraInfo;

} pin_struct;

FILE *configFile;

int nrOfLines = 0;
int nrOfPins = 0;

pin_struct *pins;

pin_struct* lineToStruct( char* line) {

  // convert config line to struct and return it as pointer
  pin_struct *newPin = malloc(sizeof (*newPin));
  if((line[0] == '#') || (line[0] == '\r') || (line[0] == '\n') ) {
    return NULL;
  }

  //char pinNameString[10];

  char dataRefString[512];

  int conversionCount = sscanf(line, "%d.%d.%4[^;];%d;%d;%f;%f;%f;%512[^;];%f;%f;%d;", &newPin->master,
                                                                                         &newPin->slave,
                                                                                         &newPin->pinNameString,
                                                                                         &newPin->ioMode,
                                                                                         &newPin->reverse,
                                                                                         &newPin->center,
                                                                                         &newPin->pinMin,
                                                                                         &newPin->pinMax,
                                                                                         dataRefString,
                                                                                         &newPin->xplaneMin,
                                                                                         &newPin->xplaneMax,
																																											   &newPin->extraInfo  );
  // find index for dataRef
  XPLMDebugString("lineToStruct\n");
  int pos = 0;
  for (int i = 0; i< 512;i++) {
    if (dataRefString[i] == '[') {
      //found index
      pos = i;
      break;
    }
  }
  //int pos = strstr(dataRefString, "[");
  XPLMDebugString("lineToStruct2\n");
  if ( pos>0) {
    int index = 0;
    sscanf(dataRefString+pos, "[%d]", &index);
    XPLMDebugString("lineToStruct3\n");
    dataRefString[pos] = '\0';
    XPLMDebugString("lineToStruct4\n");
    display("index in dataref %s %d", dataRefString, index);
    XPLMDebugString("lineToStruct5\n");
    newPin->dataRefIndex = index;
  }else {
    XPLMDebugString("lineToStruct6\n");
    newPin->dataRefIndex = 0;
  }
  XPLMDebugString("lineToStruct7 index");
  //int conversionCount = sscanf(line, "%d.%d.%4[^;];%31[^;];%d;%f;%f;%f;", &newPin->master, &newPin->slave, pinNameString, ioTypeString, &newPin->reverse, &newPin->center, &newPin->pinMin, &newPin->pinMax);
  if(conversionCount != 12) {
      display("Error! converting config line %s", line);
      return NULL;
  } else {
    //display("master %d slave %d %s %d %d %f %f %f %s ", newPin->master, newPin->slave, &newPin->pinNameString, &newPin->ioMode, newPin->reverse, newPin->center, newPin->pinMin, newPin->pinMax, dataRefString);

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

#if defined(WINDOWS) || defined(WINDOWS64)
/*
 POSIX getline replacement for non-POSIX systems (like Windows)
 Differences:
     - the function returns int64_t instead of ssize_t
     - does not accept NUL characters in the input file
 Warnings:
     - the function sets EINVAL, ENOMEM, EOVERFLOW in case of errors. The above are not defined by ISO C17,
     but are supported by other C compilers like MSVC
 */
 int64_t getline(char **restrict line, size_t *restrict len, FILE *restrict fp) {
     // Check if either line, len or fp are NULL pointers
     if(line == NULL || len == NULL || fp == NULL) {
         errno = EINVAL;
         return -1;
     }

     // Use a chunk array of 128 bytes as parameter for fgets
     char chunk[128];

     // Allocate a block of memory for *line if it is NULL or smaller than the chunk array
     if(*line == NULL || *len < sizeof(chunk)) {
         *len = sizeof(chunk);
         if((*line = malloc(*len)) == NULL) {
             errno = ENOMEM;
             return -1;
         }
     }

     // "Empty" the string
     (*line)[0] = '\0';

     while(fgets(chunk, sizeof(chunk), fp) != NULL) {
         // Resize the line buffer if necessary
         size_t len_used = strlen(*line);
         size_t chunk_used = strlen(chunk);

         if(*len - len_used < chunk_used) {
             // Check for overflow
             if(*len > SIZE_MAX / 2) {
                 errno = EOVERFLOW;
                 return -1;
             } else {
                 *len *= 2;
             }

             if((*line = realloc(*line, *len)) == NULL) {
                 errno = ENOMEM;
                 return -1;
             }
         }

         // Copy the chunk to the end of the line buffer
         memcpy(*line + len_used, chunk, chunk_used);
         len_used += chunk_used;
         (*line)[len_used] = '\0';

         // Check if *line contains '\n', if yes, return the *line length
         if((*line)[len_used - 1] == '\n') {
             return len_used;
         }
     }

     return -1;
 }
#endif


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

    //display("lines in config file %d", nrOfLines);

    pins = malloc(nrOfLines * sizeof(pin_struct));

    if ((configFile = fopen("Resources/plugins/openSimIO/config.txt","r")) == NULL){
         display("Error! opening configfile");

    } else {
      char * line = NULL;
      size_t len = 0;
      ssize_t read;
      while ((read = getline(&line, &len, configFile)) != -1) {
        //display("%s", line);
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

float map(float s, float a1, float a2, float b1, float b2)
{
    return b1 + (s-a1)*(b2-b1)/(a2-a1);
}


float mapValue(float value, float min, float max, float center, float outMin, float outMax, int reverse) {
  float out = 0.0;
  float outCenter = (outMin + outMax) /2;

  if (value>max) {
    value = max;
  }
  if (value<min) {
    value = min;
  }


  if (reverse == 1) {
    if (value>=center) {
      out = map(value, center, max, outCenter, outMin);

    } else {
      out = map(value, min, center, outMax, outCenter);
    }
  } else {
    if (value>=center) {
      out = map(value, center, max, outCenter, outMax);

    } else {
      out = map(value, min, center, outMin, outCenter);
    }
  }




  if (out>outMax) {
    out = outMax;
  }
  if (out<outMin) {
    out = outMin;
  }
  return out;
}
void setAnalogData(int i, int value) {
  //display("setAnalogData ");

      int type = XPLMGetDataRefTypes(pins[i].dataRef);

      if (type == xplmType_Int) {
        int setValue[1];
        setValue[0] = (int) value;

        XPLMSetDatavi(pins[i].dataRef,setValue, pins[i].dataRefIndex, 1);
      } else if (type == xplmType_Float) {
        float setValue = mapValue(value, pins[i].pinMin, pins[i].pinMax, pins[i].center, pins[i].xplaneMin, pins[i].xplaneMax, pins[i].reverse);
        //display("setAnalogData setting float %s %f %d %d ", pins[i].pinNameString, setValue, value, pins[i].dataRefIndex);
        XPLMSetDataf(pins[i].dataRef,setValue);
      } else if (type == xplmType_Double) {
        double setValue[1];
        setValue[0] = (double) mapValue(value, pins[i].pinMin, pins[i].pinMax, pins[i].center, pins[i].xplaneMin, pins[i].xplaneMax, pins[i].reverse);
        //display("setAnalogData setting double %s %f", pinName, setValue);
        XPLMSetDatad(pins[i].dataRef,setValue[0]);
      } else if (type == xplmType_FloatArray) {
        float setValue[1];
        setValue[0] = mapValue(value, pins[i].pinMin, pins[i].pinMax, pins[i].center, pins[i].xplaneMin, pins[i].xplaneMax, pins[i].reverse);
        //display("setAnalogData[] setting float %s %f %d %d ", pins[i].pinNameString, setValue[0], value, pins[i].dataRefIndex);
        XPLMSetDatavf(pins[i].dataRef,setValue, pins[i].dataRefIndex, 1);
      }

}


void setDigitalData(int i, int value) {


  //XPLMDebugString("setDigitalData\n");

	int type = XPLMGetDataRefTypes(pins[i].dataRef);

	if (type == xplmType_Int) {

    //XPLMDebugString("setDigitalData int %d\n");
	  int setValue = value;
	  //display("setDigitalData setting int %d %d", i, setValue);
    if (pins[i].dataRef != NULL)	{
      XPLMSetDatai(pins[i].dataRef,setValue);
    }
	} else if (type == xplmType_Float) {
    //XPLMDebugString("setDigitalData float\n");
	  float setValue = mapValue(value, pins[i].pinMin, pins[i].pinMax, pins[i].center, pins[i].xplaneMin, pins[i].xplaneMax, pins[i].reverse);
	  //display("setAnalogData setting float %s %f", pinName, setValue);
	  XPLMSetDataf(pins[i].dataRef,setValue);
	} else if (type == xplmType_Double) {
	  double setValue[1];
    setValue[0] = (double) mapValue(value, pins[i].pinMin, pins[i].pinMax, pins[i].center, pins[i].xplaneMin, pins[i].xplaneMax, pins[i].reverse);
	  //display("setAnalogData setting double %s %f", pinName, setValue);
	  XPLMSetDatad(pins[i].dataRef,setValue[0]);
	} else if (type == xplmType_FloatArray) {
    //XPLMDebugString("setDigitalData float array\n");
    float setValue[1];
    setValue[0] = mapValue(value, pins[i].pinMin, pins[i].pinMax, pins[i].center, pins[i].xplaneMin, pins[i].xplaneMax, pins[i].reverse);
    //display("setAnalogData[] setting float %s %f %d %d ", pins[i].pinNameString, setValue[0], value, pins[i].dataRefIndex);
    XPLMSetDatavf(pins[i].dataRef,setValue, pins[i].dataRefIndex, 1);
  } else if (type == xplmType_IntArray) {
    //XPLMDebugString("setDigitalData int array\n");
	  int setValue[1];
    setValue[0] = (int) value;
	  //display("setDigitalData setting int %s %d", i, setValue[0]);
	  XPLMSetDatavi(pins[i].dataRef,setValue, pins[i].dataRefIndex, 1);
	}


}

void setDigitalPin(int cport_nr,int pin, int value ) {
	char out[512];

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



	for (int i=0;i<nrOfPins;i++) {
		if (pins[i].output == 1) {
			int type = XPLMGetDataRefTypes(pins[i].dataRef);

      if (type == xplmType_Int) {

        setDigitalPin(cport_nr,i,XPLMGetDatai(pins[i].dataRef) );
      } else if (type == xplmType_Float) {

      } else if (type == xplmType_Double) {

      }
		}
	}
}

void parseInputPin(char* data, int masterId, int slaveId) {
  //display("parse inputpin start  %s master %d slave %d", data, masterId, slaveId);
  char* digital = strstr(data, "D"); // this also removes leading spaces
  char* analog = strstr(data, "A");

  char pinName[6];
	int var = 0;
  int test = 0;

  if(digital != NULL) {
    //display("Found Digital %s", digital);
    test = sscanf(digital, "%4[^ ] %d", pinName, &var);
    //display("value %d , %s ", var, pinName);
    //setDigitalData(1, slaveId, pinName, var);
  }
  if(analog != NULL) {
    //display("Found Analog %s", analog);
    test = sscanf(analog, "%4[^ ] %d", pinName, &var);
    //display("value %d , %s float %f", var, pinName, ftemp);
    //setAnalogData(1, slaveId, pinName, var);

  }
  if (test == 2) {


	// find pin struct
  //display("parse inputpin %d %d %s %d ", masterId, slaveId, pinName, var);
		for (int i=0;i<nrOfPins;i++) {
			if (pins[i].master == masterId && pins[i].slave == slaveId) {
				if (strcmp(pinName, pins[i].pinNameString) == 0) {
          //display("parse inputpin found");
					switch (pins[i].ioMode) {
				    case 0:    // not configured

				      break;
						case DI_INPUT_PULLUP:    // not configured
							setDigitalData(i, var);
							break;
						case DI_INPUT_FLOATING:    // not configured
							setDigitalData(i, var);
							break;
						case AI_RAW:    // not configured
							setAnalogData(i, var);
							break;
						case AI_FILTER:    // not configured
							setAnalogData(i, var);
							break;
						case DI_ROTARY_ENCODER_TYPE1:    // not configured
							setDigitalData(i, var);
							break;
						case DI_3WAY_2:    // not configured
              //XPLMDebugString("3way switch\n");
							setAnalogData(i, var);
							break;
						}
					return;
				}
			}
		}
  }
}

int sendcount = 0;

void sendConfigToArduinoReset() {
  sendcount = 0;
}
void sendConfigToArduino(int cport_nr) {
  char out[512];

  // send digital data to arduino
  if (sendcount<nrOfPins) {
    int i = sendcount;
    sendcount++;
    //{1;2;0;D3,1,0;A2,5,3;}
    int len = sprintf(out, "{%d;%d;1;%s,%d,%d;}", pins[i].master, pins[i].slave, pins[i].pinNameString, pins[i].ioMode, pins[i].extraInfo);
    display("write serial:%s", out);
    RS232_SendBuf(cport_nr, out, len+1);
  }
}

void setDigitalPinEth(udpSocket sock,int pin, int value ) {
	char out[512];

 // send digital data to arduino
	if (pins[pin].output == 1) {
		if (pins[pin].prevValue != value) {

			int len = sprintf(out, "{%d;%d;0;%s=%d;}", pins[pin].master, pins[pin].slave, pins[pin].pinNameString, value);
			display("write udp:%s", out);

    	sendUDP(sock, out, sizeof(out));
			pins[pin].prevValue = value;
		}
	}
}
void sendDataToUDP(udpSocket sock) {

	for (int i=0;i<nrOfPins;i++) {
		if (pins[i].output == 1) {
			int type = XPLMGetDataRefTypes(pins[i].dataRef);

      if (type == xplmType_Int) {

        setDigitalPinEth(sock,i,XPLMGetDatai(pins[i].dataRef) );
      } else if (type == xplmType_Float) {

      } else if (type == xplmType_Double) {

      }
		}
	}
}
