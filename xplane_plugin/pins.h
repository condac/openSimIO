
void readConfig();
void sendConfigReset();
void sendConfigToArduino(int cport_nr);
void sendConfigToEth(udpSocket sock);
void parseInputPin(char* data, int masterId, int slaveId);
void sendDataToUDP(udpSocket sock);
void setTimeStep(float in);
void drawStatusDisplayInfo();
void setRawDataFromRef(XPLMDataRef dataRef, float value);
void setStepLoop();
void sendDataToArduino(int cport_nr);
void handleOutputs(int serial, udpSocket netsocket);

extern int useEthernet;
extern int useSerial;
