
void readConfig();
void sendConfigReset();
void sendConfigToArduino(int cport_nr);
void sendConfigToEth(udpSocket sock);
void parseInputPin(char* data, int masterId, int slaveId);
void sendDataToUDP(udpSocket sock);
void setTimeStep(float in);
void drawStatusDisplayInfo();
void setRawDataFromRef(XPLMDataRef dataRef, float value);
