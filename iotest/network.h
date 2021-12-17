#ifdef ETHERNET


#include <Ethernet.h>
#include <EthernetUdp.h>


unsigned int localPort = 34555;      // local port to listen on // Remember to use a different port if using multiple masters

// buffers for receiving and sending data
char packetBuffer[UDP_TX_PACKET_MAX_SIZE];  // buffer to hold incoming packet,
char ReplyBuffer[UDP_TX_PACKET_MAX_SIZE];        // a string to send back

// An EthernetUDP instance to let us send and receive packets over UDP
EthernetUDP Udp;

IPAddress remote;

int setupEthernet() {
  pcSerial.println("starting network");
  pcSerial.print("mac ");
  pcSerial.print(mac[0], HEX);
  pcSerial.print(":");
  pcSerial.print(mac[1], HEX);
  pcSerial.print(":");
  pcSerial.print(mac[2], HEX);
  pcSerial.print(":");
  pcSerial.print(mac[3], HEX);
  pcSerial.print(":");
  pcSerial.print(mac[4], HEX);
  pcSerial.print(":");
  pcSerial.println(mac[5], HEX);
  pcSerial.print("ip ");
  pcSerial.println(ip);
  
  
  // start the Ethernet
  Ethernet.begin(mac, ip);

  // Open serial communications and wait for port to open:
  //Serial.begin(9600);
  while (!pcSerial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // Check for Ethernet hardware present
  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    pcSerial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
    
    return 0;

  }
  
  if (Ethernet.linkStatus() == LinkOFF) {
    pcSerial.println("Ethernet cable is not connected."); // W5500 boards reports cable not connected even if it is
  }

  // start UDP
  Udp.begin(localPort);
  return 1;
}


void loopEthernet() {
  // if there's data available, read a packet
  int packetSize = Udp.parsePacket();
  if (packetSize) {
 //   Serial.print("Received packet of size ");
 //   Serial.println(packetSize);
    //Serial.print("From ");
    remote = Udp.remoteIP();
    //for (int i=0; i < 4; i++) {
    //  Serial.print(remote[i], DEC);
//      if (i < 3) {
//        Serial.print(".");
//      }
//    }
//    Serial.print(", port ");
//    Serial.println(Udp.remotePort());

    // read the packet into packetBufffer
    Udp.read(packetBuffer, UDP_TX_PACKET_MAX_SIZE);
//    Serial.println("Contents:");
    
    if (packetBuffer[0] == '*') {
      //Serial.println("clear to send!");
      cts = true;
    } else if (packetBuffer[0] == '{') {
//      Serial.println(packetBuffer);
      // find out what type of message
      char substr[10];
      int current = 1;

      current = getNextSubStr(packetBuffer, substr, current, ';');
      int masterPos = current;
      int master = atoi(substr);
  //    Serial.print("master:");
 //     Serial.println(master);
  //    Serial.println(substr);
      current = getNextSubStr(packetBuffer, substr, current, ';');
      int slave = atoi(substr);
      if (slave != myId) {
        // Slaves do not expect the master id so we need to remove it. 
        chainSerial.print("{");
        chainSerial.println(packetBuffer+masterPos);
        pcSerial.print("chain forward:");
        pcSerial.println(packetBuffer+masterPos);
      }else {
        
  //      Serial.print("slave:");
  //      Serial.println(slave);
  //      Serial.println(substr);
        current = getNextSubStr(packetBuffer, substr, current, ';');
        int what = atoi(substr);
  //      Serial.print("what:");
  //      Serial.println(what);
  //      Serial.println(substr);
        if (what == 1 || what == 2) {
          unconfigured = false;
          // set config
          current = getNextSubStr(packetBuffer, substr, current, ',');
          int pinNr = atoi(substr+1);
          if (substr[0] == 'A') {
            pinNr = pinNr+DIGITAL_PIN_COUNT;
          }
          if (substr[0] == 'M') {
            pinNr = pinNr+DIGITAL_PIN_COUNT+ANALOG_PIN_COUNT;
          }
  //        Serial.print("pinNr:");
  //        Serial.println(pinNr);
  //        Serial.println(substr);
          current = getNextSubStr(packetBuffer, substr, current, ',');
          int pinmode = atoi(substr);
  //        Serial.print("pinmode:");
  //        Serial.println(pinmode);
  //        Serial.println(substr);
          current = getNextSubStr(packetBuffer, substr, current, ';');
          int pinExtra = atoi(substr);
  //        Serial.print("pinExtra:");
  //        Serial.println(pinExtra);
  //        Serial.println(substr);
          
          setConfig(pinNr, pinmode, pinExtra);
          //wdt_reset();
          setupAllPins();
          //wdt_reset();
          
        } else if (what == 0) {
          // set value
          
          current = getNextSubStr(packetBuffer, substr, current, '=');
          int pinNr = atoi(substr+1);
          if (substr[0] == 'A') {
            pinNr = pinNr+DIGITAL_PIN_COUNT;
          }
          if (substr[0] == 'M') {
            pinNr = pinNr+DIGITAL_PIN_COUNT+ANALOG_PIN_COUNT;
          }
          current = getNextSubStr(packetBuffer, substr, current, ';');
          int value = atoi(substr);
          setValue(pinNr, value);
  //        Serial.print("set value:");
  //        Serial.println(substr);
          
        }
      }
      
    }
    else {
      Serial.println(packetBuffer);
    }
    wdt_reset();

  }

}



void sendDataEth() {
//  Serial.print("sendDataEth loops:");
//  Serial.println(loops);
  cts = false;
  bool changes = false;
  
  for (int i = 0; i<DIGITAL_PIN_COUNT+ANALOG_PIN_COUNT; i++) {
    if (pin_changed[i]) {
      changes = true;
      break;
    }
  }
  if (changes != true) {
    //Serial.print("sendDataEth: return");
    return;
  }
  char strbuf[32];

//  remote = Udp.remoteIP();
//    for (int i=0; i < 4; i++) {
//      Serial.print(remote[i], DEC);
//      if (i < 3) {
//        Serial.print(".");
//      }
//    }
//    Serial.print(", port ");
//    Serial.println(Udp.remotePort());

  Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());

  if (unconfigured) {
    Udp.write("{98;");
  }else  {
    Udp.write("{99;");
  }
  
  #ifdef MASTER
  itoa(MASTER_ID,strbuf,10);
  Udp.write(strbuf);
  Udp.write(";");
  #endif
  
  itoa(myId,strbuf,10);
  Udp.write(strbuf);
  Udp.write(";");
  for (int i = 0; i<DIGITAL_PIN_COUNT; i++) {
    if (pin_changed[i]) {
      pin_changed[i]--;
      Udp.write("D");
      itoa(i,strbuf,10);
      Udp.write(strbuf);
      Udp.write(" ");
      itoa(pinsData[i],strbuf,10);
      Udp.write(strbuf);
      Udp.write(",");
    }
  }
  for (int i = DIGITAL_PIN_COUNT; i<DIGITAL_PIN_COUNT+ANALOG_PIN_COUNT; i++) {
    if (pin_changed[i]) {
      pin_changed[i]--;
      Udp.write("A");
      itoa(i-DIGITAL_PIN_COUNT,strbuf,10);
      Udp.write(strbuf);
      Udp.write(" ");
      itoa(pinsData[i],strbuf,10);
      Udp.write(strbuf);
      Udp.write(",");
    }
  }
  for (int i = DIGITAL_PIN_COUNT+ANALOG_PIN_COUNT; i<DIGITAL_PIN_COUNT+ANALOG_PIN_COUNT+MCP_PIN_COUNT; i++) {
    if (pin_changed[i]) {
      pin_changed[i]--;
      Udp.write("M");
      itoa(i-DIGITAL_PIN_COUNT,strbuf,10);
      Udp.write(strbuf);
      Udp.write(" ");
      itoa(pinsData[i],strbuf,10);
      Udp.write(strbuf);
      Udp.write(",");
    }
  }
  Udp.write(";");
  //Udp.write(type);
  Udp.write("}");
  Udp.endPacket();
  //Udp.flush();
}

void relayToPCEth() {
  // message to PC from slave via ethernet
  char data;
  char strbuf[32];
  Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
  
  Udp.write("{99;");
  itoa(MASTER_ID,strbuf,10);
  Udp.write(strbuf);
  //Udp.write(MASTER_ID); // add master id
  //Udp.write(""); 
  while (chainSerial.available() <= 0) {}
  data = chainSerial.read();
  while (data != '}') {
    Udp.write(data);
    while (chainSerial.available() <= 0) {
      
    }
    data = chainSerial.read();
  }
  // end of message
  Udp.write(data);
  Udp.endPacket();
  return;
}

#endif // ETHERNET
