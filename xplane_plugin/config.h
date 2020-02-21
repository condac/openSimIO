#ifndef CONFIG_H_INCLUDED
#   define CONFIG_H_INCLUDED

FILE *configFile;

#   define IS_ETH 1
#   define IS_SERIAL 2
#   define MAXMASTERS 10


void readConfig();
int getNrOfLines(char *filename);
int getNrOfMasters(char *filename);
int readSerialConfig(char *port);
int readEthernetConfig(char *ip, int *port);
void createSockets();
void createSerialPorts();
void sendConfig();
void sendConfigReset();

typedef struct {

   int nr;
   int type;
   int udpPort;
   char ip[18];
   char serialport[32];
   udpSocket socket;
   int portNumber;

} master_struct;

#endif
