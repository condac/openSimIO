#include "openSimIO.h"
#include "udp.h"

#include "pins.h"
#include "config.h"

extern pin_struct *pins;

int nrOfLines = 0;
int nrOfPins = 0;

master_struct masters[MAXMASTERS];

char *filename = "Resources/plugins/MFD39/config.txt";

void readConfig() {
   nrOfLines = 0;
   nrOfPins = 0;

   nrOfLines = getNrOfLines(filename);
   int nrOfMasters = nrOfLines = getNrOfMasters(filename);
   pins = malloc(nrOfLines * sizeof(pin_struct));

   if ((configFile = fopen(filename, "r")) == NULL) {
      display("Error! opening configfile");

   } else {
      char *line = NULL;
      size_t len = 0;
      ssize_t read;
      while ((read = getline(&line, &len, configFile)) != -1) {
         //display("%s", line);
         pin_struct *newPin = lineToStruct(line);
         if (newPin != NULL) {
            memcpy(pins + nrOfPins, newPin, sizeof(pin_struct));
            nrOfPins++;
         }
      }
   }
   XPLMDebugString("MFD39: Creating sockets\n");
   createSockets();
   XPLMDebugString("MFD39: sockets Done\n");

   XPLMDebugString("MFD39: Creating serialports\n");
   createSerialPorts();
   XPLMDebugString("MFD39: Done\n");



}

int getNrOfLines(char *filename) {

   if ((configFile = fopen(filename, "r")) == NULL) {
      display("Error! opening configfile");
      return 0;
   } else {

      char *line = NULL;
      size_t len = 0;
      ssize_t read;

      while ((read = getline(&line, &len, configFile)) != -1) {
         nrOfLines++;
      }
      fclose(configFile);
      return nrOfLines;
   }
}

int getNrOfMasters(char *filename) {
   int returnMasters = 0;

   if ((configFile = fopen(filename, "r")) == NULL) {
      display("Error! opening configfile");
      return 0;
   } else {

      char *line = NULL;
      size_t len = 0;
      ssize_t read;

      while ((read = getline(&line, &len, configFile)) != -1) {
         if (line[0] == '/') {
            int nr = 0;
            char *type;
            int res = sscanf(line, "/%d;%s;", &nr, type);
            if (res == 2 && nr > 0 && nr < MAXMASTERS) {

               if (type[0] == 'n') {
                  masters[nr].type = IS_ETH;
                  int nr2;
                  res = sscanf(line, "/%d;n;%s %d/", &nr2, masters[nr].ip, &masters[nr].udpPort);
                  if (res == 3) {
                     XPLMDebugString("Found ip in config\n");
                     returnMasters++;
                  }
               } else if (type[0] == 's') {
                  masters[nr].type = IS_SERIAL;
                  int nr2;
                  res = sscanf(line, "/%d;s;%31[^;];", &nr2, masters[nr].serialport);
                  if (res == 2) {
                     XPLMDebugString("Found serial in config\n");
                     returnMasters++;
                  }
               }
            }
         }

      }
      fclose(configFile);
      return returnMasters;
   }
}

int readEthernetConfig(char *ip, int *port) {
   FILE *configFile;
   if ((configFile = fopen(filename, "r")) == NULL) {
      XPLMDebugString("Error! opening configfile\n");
      display("Error! opening configfile");
   } else {

      char *line = NULL;
      size_t len = 0;
      ssize_t read;
      XPLMDebugString("opening configfile\n");
      while ((read = getline(&line, &len, configFile)) != -1) {
         if (line[0] == '/') {

            int res = sscanf(line, "/1;n;%s %d/", ip, port);
            if (res == 2) {
               XPLMDebugString("Found ip in config\n");
               display("Found ip in config");
               XPLMDebugString("ethernet ok\n");
               return 1;
            } else {
               XPLMDebugString("ethernet error2\n");
            }

         }
      }
      fclose(configFile);
   }
   return -1;
}


int readSerialConfig(char *port) {
   FILE *configFile;
   if ((configFile = fopen(filename, "r")) == NULL) {
      XPLMDebugString("Error! opening configfile\n");
      display("Error! opening configfile");
   } else {

      char *line = NULL;
      size_t len = 0;
      ssize_t read;
      XPLMDebugString("opening configfile\n");
      while ((read = getline(&line, &len, configFile)) != -1) {
         if (line[0] == '/') {

            int res = sscanf(line, "/1;s;%31[^;];", port);
            if (res == 1) {
               XPLMDebugString("Found serial in config");
               display("Found serial in config");
               XPLMDebugString("serial ok");
               return 1;
            } else {
               XPLMDebugString("serial error2");
            }

         }
      }
      fclose(configFile);
   }
   return -1;
}

void createSockets() {
   for (int i = 0; i < MAXMASTERS; i++) {
      if (masters[i].type == IS_ETH) {

         masters[i].socket = createUDPSocket(masters[i].ip, masters[i].udpPort);

      }

   }

}
void createSerialPorts() {
   for (int i = 0; i < MAXMASTERS; i++) {
      if (masters[i].type == IS_SERIAL) {

         masters[i].socket = createUDPSocket(masters[i].ip, masters[i].serialport);
         int bdrate = 115200;   /* 115200 baud */
         char mode[] = { '8', 'N', '1', 0 };

         masters[i].portNumber = RS232_OpenComport(masters[i].serialport, bdrate, mode, 0);

         if (masters[i].portNumber == -1) {
            display("Error: Can not open comport %s\n", masters[i].serialport);

         }

      }

   }
}

int sendcount = 0;

void sendConfigReset() {
   sendcount = 0;
}
void sendConfig() {
   char out[512];

   // send digital data to arduino
   if (sendcount < nrOfPins) {
      int i = sendcount;
      sendcount++;
      //{1;2;0;D3,1,0;A2,5,3;}
      int len =
         sprintf(out, "{%d;%d;1;%s,%d,%d;}", pins[i].master, pins[i].slave, pins[i].pinNameString, pins[i].ioMode,
                 pins[i].pinExtra);

      display("write serial:%s", out);
      if (masters[pins[i].master].type == IS_SERIAL) {

         RS232_SendBuf(masters[pins[i].master].serialport, out, len + 1);
      }
      if (masters[pins[i].master].type == IS_ETH) {
         sendUDP(masters[pins[i].master].socket, out, len + 1);
      }
   }
}
