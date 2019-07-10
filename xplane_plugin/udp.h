typedef struct UDPSocket{
#ifdef _WIN32
	SOCKET sock;
#else
	int sock;
#endif
  char ip[18];
	unsigned short port;

} udpSocket;


udpSocket createUDPSocket(char* ipIn, int portIn);
int readUDP(udpSocket sock, char buffer[], int len);
int sendUDP(udpSocket socket, char buffer[], int len);
int ifMessage(udpSocket sock);
