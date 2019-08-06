#include "openSimIO.h"
#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "iotypes.h"
#include "udp.h"
#include "rs232.h"
#include "statusDisplay.h"
#include "iotypes.h"
#include "pins.h"

typedef struct  {

	int master;
  int slave;
  int ioMode;
  XPLMDataRef dataRef;
	XPLMCommandRef commandRef;
  int dataRefIndex;
	int pinExtra;
  float pinMin;
  float pinMax;
	float xplaneCenter;
  float xplaneMin;
  float xplaneMax;
	float xplaneExtra;
  float center;
  int reverse;
  int pinNr;
  char pinNameString[10];
	char dataRefString[512];
	int output;
	int prevValue;
	float prevValueF;
	float lastSimValue;

} pin_struct;

FILE *configFile;

int nrOfLines = 0;
int nrOfPins = 0;
float timeStep = 0;
float timeLast = 0;

pin_struct *pins;

pin_struct* lineToStruct( char* line) {

  // convert config line to struct and return it as pointer
  pin_struct *newPin = malloc(sizeof (*newPin));
  if((line[0] == '#') || (line[0] == '\r') || (line[0] == '\n') ) {
    return NULL;
  }
	if((line[0] == '*') ) {
		// one time dataref set
		XPLMDebugString("read *\n");
		char dataref[512];
		float value;
		int c = sscanf(line,"*%512[^;];%f;", dataref, &value);
		if (c == 2) {
			XPLMDebugString("find dataref *\n");
			XPLMDataRef dataRef = XPLMFindDataRef(dataref);

	    if (dataRef != NULL)	{
				XPLMDebugString("set onetime");
				setRawDataFromRef(dataRef, value);
				display("overiding %s %f",dataref, value );
			}
		} else {
			XPLMDebugString("error *\n");
		}

    return NULL;
  }

  //char pinNameString[10];

  char modeString[128];

  int conversionCount = sscanf(line, "%d.%d.%4[^;];%d;%128[^;];%d;%f;%f;%f;%512[^;];%f;%f;%f;%f;", &newPin->master,
                                                                                         &newPin->slave,
                                                                                         &newPin->pinNameString,
																																												 &newPin->pinExtra,
                                                                                         modeString,
                                                                                         &newPin->reverse,
                                                                                         &newPin->center,
                                                                                         &newPin->pinMin,
                                                                                         &newPin->pinMax,
                                                                                         &newPin->dataRefString,
																																												 &newPin->xplaneCenter,
                                                                                         &newPin->xplaneMin,
                                                                                         &newPin->xplaneMax,
																																											   &newPin->xplaneExtra  );

  // Translate iomode from string to int
	newPin->ioMode =  getTypeFromString(modeString);
	//display("iomode %s %d",modeString, newPin->ioMode  );

  // find index for dataRef

  int pos = 0;
  for (int i = 0; i< 512;i++) {
    if (newPin->dataRefString[i] == '[') {
      //found index
      pos = i;
      break;
    }
  }
  //int pos = strstr(dataRefString, "[");

  if ( pos>0) {
    int index = 0;
    sscanf(newPin->dataRefString+pos, "[%d]", &index);

    newPin->dataRefString[pos] = '\0';

    //display("index in dataref %s %d", dataRefString, index);

    newPin->dataRefIndex = index;
  }else {

    newPin->dataRefIndex = 0;
  }

  //int conversionCount = sscanf(line, "%d.%d.%4[^;];%31[^;];%d;%f;%f;%f;", &newPin->master, &newPin->slave, pinNameString, ioTypeString, &newPin->reverse, &newPin->center, &newPin->pinMin, &newPin->pinMax);
  if(conversionCount != 14) {
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
		newPin->dataRef = XPLMFindDataRef(newPin->dataRefString);

    if (newPin->dataRef == NULL)	{
			newPin->commandRef = XPLMFindCommand(newPin->dataRefString);
			if (newPin->commandRef == NULL)	{
	      display("dataRef invalid %s", newPin->dataRefString);
	      return NULL;
			}
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
	nrOfLines = 0;
	nrOfPins = 0;
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


float mapValue(float value, float min, float max, float center, float outMin, float outMax, int reverse, float extra, float outCenter) {
  float out = 0.0;
  //float outCenter = (outMin + outMax) /2;
	if (extra <= 0.01) {
		extra = 1.0;
	}
  if (value>max) {
    value = max;
  }
  if (value<min) {
    value = min;
  }


  if (reverse == 1) {
    if (value>=center) {
			// For exponential convertion to work we must first map the input from 0-1 then apply exp and then scale to final range
			out = map(value, center, max, 0, 1);
			out = pow(out, extra);
			out = map(out, 0, 1, outCenter, outMin);

    } else {
			out = map(value, min, center, 1, 0);
			out = pow(out, extra);
      out = map(out, 1, 0, outMax, outCenter);
    }
  } else {
    if (value>=center) {
			out = map(value, center, max, 0, 1);
			out = pow(out, extra);
      out = map(out, 0, 1, outCenter, outMax);

    } else {
			out = map(value, min, center, 1, 0);
			out = pow(out, extra);
      out = map(out, 1, 0, outMin, outCenter);
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
void setAnalogData(int i, float value) {
	pins[i].prevValue = value;
  //display("setAnalogData ");

      int type = XPLMGetDataRefTypes(pins[i].dataRef);

      if (type == xplmType_Int) {
        int setValue;
        setValue = (int) value;

        XPLMSetDatai(pins[i].dataRef,setValue);
				pins[i].lastSimValue = setValue;
      } else if (type == xplmType_IntArray) {
        int setValue[1];
        setValue[0] = (int) value;

        XPLMSetDatavi(pins[i].dataRef,setValue, pins[i].dataRefIndex, 1);
				pins[i].lastSimValue = setValue[0];
      } else if (type == xplmType_Float) {
        float setValue = mapValue(value, pins[i].pinMin, pins[i].pinMax, pins[i].center, pins[i].xplaneMin, pins[i].xplaneMax, pins[i].reverse, pins[i].xplaneExtra, pins[i].xplaneCenter);
        //display("setAnalogData setting float %s %f %d %d ", pins[i].pinNameString, setValue, value, pins[i].dataRefIndex);
        XPLMSetDataf(pins[i].dataRef,setValue);
				pins[i].lastSimValue = setValue;
      } else if (type == xplmType_Double) {
        double setValue[1];
        setValue[0] = (double) mapValue(value, pins[i].pinMin, pins[i].pinMax, pins[i].center, pins[i].xplaneMin, pins[i].xplaneMax, pins[i].reverse, pins[i].xplaneExtra, pins[i].xplaneCenter);
        //display("setAnalogData setting double %s %f", pinName, setValue);
        XPLMSetDatad(pins[i].dataRef,setValue[0]);
				pins[i].lastSimValue = setValue[0];
      } else if (type == xplmType_FloatArray) {
        float setValue[1];
        setValue[0] = mapValue(value, pins[i].pinMin, pins[i].pinMax, pins[i].center, pins[i].xplaneMin, pins[i].xplaneMax, pins[i].reverse, pins[i].xplaneExtra, pins[i].xplaneCenter);
        //display("setAnalogData[] setting float %s %f %d %d ", pins[i].pinNameString, setValue[0], value, pins[i].dataRefIndex);
        XPLMSetDatavf(pins[i].dataRef,setValue, pins[i].dataRefIndex, 1);
				pins[i].lastSimValue = setValue[0];
      }

}
void setRawDataF(int i, float value) {
  //display("setRawDataF %f", value);

      int type = XPLMGetDataRefTypes(pins[i].dataRef);

			if (type == xplmType_Int) {
        int setValue;
        setValue = (int) value;

        XPLMSetDatai(pins[i].dataRef,setValue);
				pins[i].lastSimValue = setValue;
      } else if (type == xplmType_IntArray) {
        int setValue[1];
        setValue[0] = (int) value;

        XPLMSetDatavi(pins[i].dataRef,setValue, pins[i].dataRefIndex, 1);
				pins[i].lastSimValue = setValue[0];
      } else if (type == xplmType_Float) {
        //display("setAnalogData setting float %s %f %d %d ", pins[i].pinNameString, setValue, value, pins[i].dataRefIndex);
        XPLMSetDataf(pins[i].dataRef,value);
				pins[i].lastSimValue = value;
      } else if (type == xplmType_Double) {
        double setValue;
        setValue = (double) value;
        //display("setAnalogData setting double %s %f", pinName, setValue);
        XPLMSetDatad(pins[i].dataRef,setValue);
				pins[i].lastSimValue = setValue;
      } else if (type == xplmType_FloatArray) {
        float setValue[1];
        setValue[0] = value;
        //display("setAnalogData[] setting float %s %f %d %d ", pins[i].pinNameString, setValue[0], value, pins[i].dataRefIndex);
        XPLMSetDatavf(pins[i].dataRef,setValue, pins[i].dataRefIndex, 1);
				pins[i].lastSimValue = setValue[0];
      }

}

void setRawDataFromRef(XPLMDataRef dataRef, float value) {
  //display("setRawDataF %f", value);

      int type = XPLMGetDataRefTypes(dataRef);

			if (type == xplmType_Int) {
        int setValue;
        setValue = (int) value;
				display("setRawDataFromRef %d %f", setValue, value);
        XPLMSetDatai(dataRef,setValue);

      } else if (type == xplmType_IntArray) {
        int setValue[1];
        setValue[0] = (int) value;

        XPLMSetDatavi(dataRef,setValue, 0, 1);

      } else if (type == xplmType_Float) {
        //display("setAnalogData setting float %s %f %d %d ", pins[i].pinNameString, setValue, value, pins[i].dataRefIndex);
        XPLMSetDataf(dataRef,value);

      } else if (type == xplmType_Double) {
        double setValue;
        setValue = (double) value;
        //display("setAnalogData setting double %s %f", pinName, setValue);
        XPLMSetDatad(dataRef,setValue);

      } else if (type == xplmType_FloatArray) {
        float setValue[1];
        setValue[0] = value;
        //display("setAnalogData[] setting float %s %f %d %d ", pins[i].pinNameString, setValue[0], value, pins[i].dataRefIndex);
        XPLMSetDatavf(dataRef,setValue, 0, 1);

      }

}

float getRawDataF(int i) {

      int type = XPLMGetDataRefTypes(pins[i].dataRef);

      if (type == xplmType_Int) {

        return (float)XPLMGetDatai(pins[i].dataRef);
      } else if (type == xplmType_Float) {
        //display("setAnalogData setting float %s %f %d %d ", pins[i].pinNameString, setValue, value, pins[i].dataRefIndex);
        return XPLMGetDataf(pins[i].dataRef);
      } else if (type == xplmType_Double) {
        //display("setAnalogData setting double %s %f", pinName, setValue);
        XPLMGetDatad(pins[i].dataRef);
      } else if (type == xplmType_FloatArray) {
        float readValue[1];

        //display("setAnalogData[] setting float %s %f %d %d ", pins[i].pinNameString, setValue[0], value, pins[i].dataRefIndex);
        XPLMGetDatavf(pins[i].dataRef,readValue, pins[i].dataRefIndex, 1);
				return readValue[0];
      }
	return -1;
}

void setTimeStep(float in) {

	timeStep = in - timeLast;
	timeLast = in;
}

void setStepData(int pin, int value) {
	// increse step while holding button based on time elapsed
	// the center value in configuration is the steps per second we want to create
	pins[pin].prevValue = value;


}

void setStepLoop() {
	for (int i=0;i<nrOfPins;i++) {
		if (pins[i].ioMode == DI_INPUT_STEP || (pins[i].ioMode == DI_4X4 && pins[i].xplaneExtra == 2)) {
			if (pins[i].prevValue == 1) {
			// increse step while holding button based on time elapsed
			// the center value in configuration is the steps per second we want to create

				// Read current data
				float current = getRawDataF(i);
				float out = pins[i].center * timeStep;
				if (pins[i].reverse == 1) {
					current = current - out;
				} else {
					current = current + out;
				}
				if (current > pins[i].xplaneMax) {
					current = pins[i].xplaneMax;
				}
				if (current < pins[i].xplaneMin) {
					current = pins[i].xplaneMin;
				}
				setRawDataF(i, current);

			}
		}

	}
}

void setDigitalData(int i, int value) {

	pins[i].prevValue = value;
  //XPLMDebugString("setDigitalData\n");

	int type = XPLMGetDataRefTypes(pins[i].dataRef);

	if (type == xplmType_Int) {

    //XPLMDebugString("setDigitalData int %d\n");
	  int setValue = value;
	  //display("setDigitalData setting int %d %d", i, setValue);
    if (pins[i].dataRef != NULL)	{
      XPLMSetDatai(pins[i].dataRef,setValue);
    }
		pins[i].lastSimValue = setValue;
	} else if (type == xplmType_Float) {
    //XPLMDebugString("setDigitalData float\n");
	  float setValue = mapValue(value, pins[i].pinMin, pins[i].pinMax, pins[i].center, pins[i].xplaneMin, pins[i].xplaneMax, pins[i].reverse, 1, pins[i].xplaneCenter);
	  //display("setAnalogData setting float %s %f", pinName, setValue);
	  XPLMSetDataf(pins[i].dataRef,setValue);
		pins[i].lastSimValue = setValue;
	} else if (type == xplmType_Double) {
	  double setValue[1];
    setValue[0] = (double) mapValue(value, pins[i].pinMin, pins[i].pinMax, pins[i].center, pins[i].xplaneMin, pins[i].xplaneMax, pins[i].reverse, 1, pins[i].xplaneCenter);
	  //display("setAnalogData setting double %s %f", pinName, setValue);
	  XPLMSetDatad(pins[i].dataRef,setValue[0]);
		pins[i].lastSimValue = setValue[0];
	} else if (type == xplmType_FloatArray) {
    //XPLMDebugString("setDigitalData float array\n");
    float setValue[1];
    setValue[0] = mapValue(value, pins[i].pinMin, pins[i].pinMax, pins[i].center, pins[i].xplaneMin, pins[i].xplaneMax, pins[i].reverse, 1, pins[i].xplaneCenter);
    //display("setAnalogData[] setting float %s %f %d %d ", pins[i].pinNameString, setValue[0], value, pins[i].dataRefIndex);
    XPLMSetDatavf(pins[i].dataRef,setValue, pins[i].dataRefIndex, 1);
		pins[i].lastSimValue = setValue[0];
  } else if (type == xplmType_IntArray) {
    //XPLMDebugString("setDigitalData int array\n");
	  int setValue[1];
    setValue[0] = (int) value;
	  //display("setDigitalData setting int %s %d", i, setValue[0]);
	  XPLMSetDatavi(pins[i].dataRef,setValue, pins[i].dataRefIndex, 1);
		pins[i].lastSimValue = setValue[0];
	}


}



void digitalButton(int i, int var) {
	if (pins[i].xplaneExtra == 0) {
		// on off switch
		if (pins[i].dataRef != NULL) {
			int set = 0;
			if (pins[i].reverse == 1) {
				if (var == 1) {
					set = pins[i].xplaneMin;
				} else {
					set = pins[i].xplaneMax;
				}
			}else {
				if (var == 1) {
					set = pins[i].xplaneMax;
				} else {
					set = pins[i].xplaneMin;
				}
			}
			setDigitalData(i, set);
		}else if (pins[i].commandRef != NULL) {
			if (pins[i].reverse == 1) {
				if (var == 1) {
					XPLMCommandEnd(pins[i].commandRef);
				} else {
					XPLMCommandBegin(pins[i].commandRef);
				}
			}else {
				if (var == 1) {
					XPLMCommandBegin(pins[i].commandRef);
				} else {
					XPLMCommandEnd(pins[i].commandRef);
				}
			}

		}

	} else if (pins[i].xplaneExtra == 1) {
		// toggle data
		if (1 == var) {
			if (pins[i].prevValue == 0) {
				pins[i].prevValue = 1;
				if (pins[i].dataRef != NULL) {
					float current = getRawDataF(i);
					if (current > pins[i].xplaneMin) {
						setRawDataF(i, pins[i].xplaneMin);
					} else {
						setRawDataF(i, pins[i].xplaneMax);
					}
				} else if (pins[i].commandRef != NULL) {
					XPLMCommandOnce(pins[i].commandRef);
				}


			}

		} else if(var == 0) {
			pins[i].prevValue = 0;
			//setDigitalData(i, 0);

		}
	}else if (pins[i].xplaneExtra == 2) {
		// step data
		if (pins[i].pinExtra == var) {
			setStepData(i, 1);

		} else if(var == 0) {
			setStepData(i, 0);

		} else {

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
						case DI_INPUT_PULLUP:    //
							digitalButton(i, var);

							break;
						case DI_INPUT_FLOATING:    //
							digitalButton(i, var);
							break;
						case DI_INPUT_STEP:    //
							setStepData(i, var);
							break;

						case AI_RAW:    //
							setAnalogData(i, var);
							break;
						case AI_FILTER:    //
							setAnalogData(i, var);
							break;
						case AI_OVERSAMPLE:    //
							setAnalogData(i, ((float)var)/10);
							break;
						case DI_ROTARY_ENCODER_TYPE1:    //
							setDigitalData(i, var * pins[i].center);
							break;
						case DI_3WAY_2:    //
              //XPLMDebugString("3way switch\n");
							setAnalogData(i, var);
							break;
						case DI_4X4:    //
							// this might go in its own function, but then we cant use the continue to go forward in the for loop
							if (pins[i].xplaneExtra == 0) {
								if (pins[i].pinExtra == var) {
									setDigitalData(i, 1);
								} else if(var == 0) {
									setDigitalData(i, 0);
									continue;
								} else {
									continue;
								}
							} else if (pins[i].xplaneExtra == 1) {
								// toggle data
								if (pins[i].pinExtra == var) {
									if (pins[i].prevValue == 0) {
										pins[i].prevValue = 1;
										if (pins[i].dataRef != NULL) {
											float current = getRawDataF(i);
											if (current > pins[i].xplaneMin) {
												setRawDataF(i, pins[i].xplaneMin);
											} else {
												setRawDataF(i, pins[i].xplaneMax);
											}
										} else if (pins[i].commandRef != NULL) {
											XPLMCommandOnce(pins[i].commandRef);
										}


									}

								} else if(var == 0) {
									pins[i].prevValue = 0;
									//setDigitalData(i, 0);
									continue;
								} else {
									continue;
								}
							}else if (pins[i].xplaneExtra == 2) {
								// toggle data
								if (pins[i].pinExtra == var) {
									setStepData(i, 1);

								} else if(var == 0) {
									setStepData(i, 0);
									continue;
								} else {
									continue;
								}
							}

							break;
						}
					//return; // If we do not return we can have multiple functions on same input, for example tire steer and rudder on same axis
				}
			}
		}
  }
}

int sendcount = 0;

void sendConfigReset() {
  sendcount = 0;
}
void sendConfigToArduino(int cport_nr) {
  char out[512];

  // send digital data to arduino
  if (sendcount<nrOfPins) {
    int i = sendcount;
    sendcount++;
    //{1;2;0;D3,1,0;A2,5,3;}
    int len = sprintf(out, "{%d;%d;1;%s,%d,%d;}", pins[i].master, pins[i].slave, pins[i].pinNameString, pins[i].ioMode, pins[i].pinExtra);
    display("write serial:%s", out);
    RS232_SendBuf(cport_nr, out, len+1);
  }
}
void sendConfigToEth(udpSocket sock) {
  char out[512];

  // send digital data to arduino
  if (sendcount<nrOfPins) {
    int i = sendcount;
    sendcount++;
    //{1;2;0;D3,1,0;A2,5,3;}
    int len = sprintf(out, "{%d;%d;1;%s,%d,%d;}", pins[i].master, pins[i].slave, pins[i].pinNameString, pins[i].ioMode, pins[i].pinExtra);
    //display("write udp:%s", out);
		sendUDP(sock, out, len+1);
    //RS232_SendBuf(cport_nr, out, len+1);
  }
}


void setDigitalPinSerial(int cport_nr,int pin, int value ) {
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

void setDigitalPinEth(udpSocket sock,int pin, int value ) {
	char out[512];

 // send digital data to arduino
	if (pins[pin].output == 1) {
		if (pins[pin].prevValue != value) {

			int len = sprintf(out, "{%d;%d;0;%s=%d;}", pins[pin].master, pins[pin].slave, pins[pin].pinNameString, value);
			//display("write udp:%s", out);

    	sendUDP(sock, out, sizeof(out));
			pins[pin].prevValue = value;
		}
	}
}
void sendDataToUDP(udpSocket sock) {

	for (int i=0;i<nrOfPins;i++) {
		if (pins[i].output == 1) {
			if (pins[i].ioMode == DO_HIGH || pins[i].ioMode == DO_LOW) {
				// do nothing
				continue;
			}
			int type = XPLMGetDataRefTypes(pins[i].dataRef);
			float outValue = 0.0;
      if (type == xplmType_Int) {
				int temp = XPLMGetDatai(pins[i].dataRef);
				outValue = temp;
        //setDigitalPinEth(sock,i, temp);

      } else if (type == xplmType_Float) {
				outValue = XPLMGetDataf(pins[i].dataRef);


      } else if (type == xplmType_FloatArray) {
				float readValue[1];
        XPLMGetDatavf(pins[i].dataRef,readValue, pins[i].dataRefIndex, 1);
				outValue = readValue[0];


      } else if (type == xplmType_Double) {

      }
			pins[i].lastSimValue = outValue;
			// Transform value
			int outValueInt = map(outValue, pins[i].xplaneMin, pins[i].xplaneMax, pins[i].pinMin, pins[i].pinMax);

			// if ethernet or serial
			setDigitalPinEth(sock,i,outValueInt);

		}
	}
}
void handleOutputs(int serial, udpSocket netsocket) {
	for (int i=0;i<nrOfPins;i++) {
		if (pins[i].output == 1) {
			if (pins[i].ioMode == DO_HIGH || pins[i].ioMode == DO_LOW) {
				// do nothing
				continue;
			}
			int type = XPLMGetDataRefTypes(pins[i].dataRef);
			float outValue = 0.0;
			if (type == xplmType_Int) {
				int temp = XPLMGetDatai(pins[i].dataRef);
				outValue = temp;
				//setDigitalPinEth(sock,i, temp);

			} else if (type == xplmType_Float) {
				outValue = XPLMGetDataf(pins[i].dataRef);


			} else if (type == xplmType_FloatArray) {
				float readValue[1];
				XPLMGetDatavf(pins[i].dataRef,readValue, pins[i].dataRefIndex, 1);
				outValue = readValue[0];


			} else if (type == xplmType_Double) {

			}
			pins[i].lastSimValue = outValue;
			// Transform value
			//int outValueInt = map(outValue, pins[i].xplaneMin, pins[i].xplaneMax, pins[i].pinMin, pins[i].pinMax);
			//outValueInt = mapValue(outValue, pins[i].pinMin, pins[i].pinMax, pins[i].center, pins[i].xplaneMin, pins[i].xplaneMax, pins[i].reverse, pins[i].xplaneExtra, pins[i].xplaneCenter);
			int outValueInt = mapValue(outValue, pins[i].xplaneMin, pins[i].xplaneMax, pins[i].xplaneCenter, pins[i].pinMin, pins[i].pinMax, pins[i].reverse, pins[i].xplaneExtra, pins[i].center);

			switch (pins[i].ioMode) {
				case DO_BOOL:    //

					if (outValue > pins[i].xplaneMax) {
						// turn light off if it is greater than xplaneMax

						if (pins[i].reverse == 1) {
							outValueInt = pins[i].pinMax;
						} else {
							outValueInt = pins[i].pinMin;
						}
					} else if (outValue > pins[i].xplaneMin) {
						// Turn light on if above xplaneMax
						if (pins[i].reverse == 1) {
							outValueInt = pins[i].pinMin;
						} else {
							outValueInt = pins[i].pinMax;
						}
					}
					else {
						if (pins[i].reverse == 1) {
							outValueInt = pins[i].pinMax;
						} else {
							outValueInt = pins[i].pinMin;
						}
					}

					break;
				default:
					break;
			}
			// if ethernet or serial
			if (useSerial == 1) {
				setDigitalPinSerial(serial,i,outValueInt);
			}
			if (useEthernet == 1) {
				setDigitalPinEth(netsocket,i,outValueInt);
			}

		}
	}
}

void drawStatusDisplayInfo() {
	statusClear();
	for (int i=0;i<nrOfPins;i++) {
		if (pins[i].output == 1) {
			statusPrintf("%s = %d <- %s=%f \n", pins[i].pinNameString, pins[i].prevValue, pins[i].dataRefString, pins[i].lastSimValue);
		} else {
			statusPrintf("%s = %d -> %s=%f \n", pins[i].pinNameString, pins[i].prevValue, pins[i].dataRefString, pins[i].lastSimValue);
		}
	}
}
