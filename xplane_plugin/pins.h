
void readConfig();
void sendConfigReset();
void sendConfigToArduino(int cport_nr);
void parseInputPin(char* data, int masterId, int slaveId);
void sendDataToUDP(udpSocket sock);
