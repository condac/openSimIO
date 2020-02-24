#define _GNU_SOURCE
#include <stdio.h>
#include "openSimIO.h"
#include "udp.h"
#include "rs232.h"
#include "pins.h"
#include "config.h"



//master_struct masters[MAXMASTERS];

char *filename = "Resources/plugins/openSimIO/config.txt";

void readConfig() {
   //masters = malloc(MAXMASTERS * sizeof(master_struct));
   for (int i = 0; i < MAXMASTERS; i++) {
      if (masters[i].type == IS_ETH) {
         closeSocket(masters[i].socket);
      }
      masters[i].type = 0;
   }
   nrOfLines = 0;
   nrOfPins = 0;
   XPLMDebugString("openSimIO.readConfig: getNrOfLines\n");
   nrOfLines = getNrOfLines(filename);
   char test[100];
   sprintf(test, "%d", nrOfLines);
   XPLMDebugString(test);
   XPLMDebugString("openSimIO.readConfig: getNrOfmasters\n");
   int nrOfMasters = getNrOfMasters(filename);
   nrOfMasters = nrOfMasters; // unused
   pins = malloc(nrOfLines * sizeof(pin_struct));

   if ((configFile = fopen(filename, "r")) == NULL) {
      XPLMDebugString("openSimIO.readConfig: error opening configfile\n");
      display("Error! opening configfile");

   } else {
      char *line = NULL;
      size_t len = 0;
      ssize_t read;
      while ((read = getline(&line, &len, configFile)) != -1) {
         //display("%s", line);
         XPLMDebugString("openSimIO.readConfig: lineToStruct\n");
         XPLMDebugString(line);
         pin_struct *newPin = lineToStruct(line);
         if (newPin != NULL) {
            XPLMDebugString("openSimIO.readConfig: lineToStruct memcpy\n");
            memcpy(pins + nrOfPins, newPin, sizeof(pin_struct));
            nrOfPins++;
         }
      }
   }
   XPLMDebugString("openSimIO:openSimIO: Creating sockets\n");
   createSockets();
   XPLMDebugString("openSimIO:openSimIO: sockets Done\n");

   XPLMDebugString("openSimIO:openSimIO: Creating serialports\n");
   createSerialPorts();
   XPLMDebugString("openSimIO:openSimIO: Done\n");



}

int getNrOfLines(char *filename) {
   int banan = 0;
   if ((configFile = fopen(filename, "r")) == NULL) {
      display("Error! opening configfile");
      return 0;
   } else {

      char *line = NULL;
      size_t len = 0;
      ssize_t read;

      while ((read = getline(&line, &len, configFile)) != -1) {
         banan++;
      }
      fclose(configFile);
      return banan;
   }
   return banan;
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
            XPLMDebugString("openSimIO.getNrOfMasters: line start with / \n");
            int nr = 0;

            //int res = sscanf(line, "/%d;%s;%*s", &nr, type);
            nr = atoi(&line[1]);

            if (nr > 0 && nr < MAXMASTERS) {
               XPLMDebugString("openSimIO.getNrOfMasters: found config line / \n");
               if (line[3] == 'n') {
                  masters[nr].type = IS_ETH;
                  int nr2;
                  int res2 = sscanf(line, "/%d;n;%s %d/", &nr2, masters[nr].ip, &masters[nr].udpPort);
                  if (res2 == 3) {
                     XPLMDebugString("openSimIO:Found ip in config\n");
                     returnMasters++;
                  }
               } else if (line[3] == 's') {
                  masters[nr].type = IS_SERIAL;
                  int nr2;
                  int res2 = sscanf(line, "/%d;s;%31[^;];", &nr2, masters[nr].serialport);
                  if (res2 == 2) {
                     XPLMDebugString("openSimIO:Found serial in config\n");
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
      XPLMDebugString("openSimIO:Error! opening configfile\n");
      display("Error! opening configfile");
   } else {

      char *line = NULL;
      size_t len = 0;
      ssize_t read;
      XPLMDebugString("openSimIO:opening configfile\n");
      while ((read = getline(&line, &len, configFile)) != -1) {
         if (line[0] == '/') {

            int res = sscanf(line, "/1;n;%s %d/", ip, port);
            if (res == 2) {
               XPLMDebugString("openSimIO:Found ip in config\n");
               display("Found ip in config");
               XPLMDebugString("openSimIO:ethernet ok\n");
               return 1;
            } else {
               XPLMDebugString("openSimIO:ethernet error2\n");
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
      XPLMDebugString("openSimIO:Error! opening configfile\n");
      display("Error! opening configfile");
   } else {

      char *line = NULL;
      size_t len = 0;
      ssize_t read;
      XPLMDebugString("openSimIO:opening configfile\n");
      while ((read = getline(&line, &len, configFile)) != -1) {
         if (line[0] == '/') {

            int res = sscanf(line, "/1;s;%31[^;];", port);
            if (res == 1) {
               XPLMDebugString("openSimIO:Found serial in config");
               display("Found serial in config");
               XPLMDebugString("openSimIO:serial ok");
               return 1;
            } else {
               XPLMDebugString("openSimIO:serial error2");
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
         XPLMDebugString("openSimIO: created socket");
      }

   }

}
void createSerialPorts() {
   for (int i = 0; i < MAXMASTERS; i++) {
      if (masters[i].type == IS_SERIAL) {


         int bdrate = 115200;   /* 115200 baud */
         char mode[] = { '8', 'N', '1', 0 };

         masters[i].portNumber = RS232_OpenComport(masters[i].serialport, bdrate, mode, 0);
         XPLMDebugString("openSimIO: created port");
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

      display("write config:%s %s", out, masters[pins[i].master].ip);
      if (masters[pins[i].master].type == IS_SERIAL) {

         RS232_SendBuf(masters[pins[i].master].portNumber, out, len + 1);
      }
      if (masters[pins[i].master].type == IS_ETH) {
         sendUDP(masters[pins[i].master].socket, out, len + 1);
      }
   }
}
