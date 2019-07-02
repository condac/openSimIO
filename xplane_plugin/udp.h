typedef struct UDPSocket{
#ifdef _WIN32
	SOCKET sock;
#else
	int sock;
#endif
  char ip[17];
	unsigned short port;

} udpSocket;


udpSocket createUDPSocket(char* ipIn, int portIn);
