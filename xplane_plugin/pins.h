#ifndef PINS_H_INCLUDED
#   define PINS_H_INCLUDED
void readConfig();
void sendConfigReset();
void sendConfigToArduino(int cport_nr);
void sendConfigToEth(udpSocket sock);
void parseInputPin(char *data, int masterId, int slaveId);
void sendDataToUDP(udpSocket sock);
void setTimeStep(float in);
void drawStatusDisplayInfo();
void setRawDataFromRef(XPLMDataRef dataRef, float value);
void setStepLoop();
void sendDataToArduino(int cport_nr);
void handleOutputs();


typedef struct {

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

pin_struct *lineToStruct(char *line);
#endif
