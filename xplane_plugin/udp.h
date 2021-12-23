#ifndef UDP_H_INCLUDED
#define UDP_H_INCLUDED

typedef struct UDPSocket {
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
void closeSocket(udpSocket sock);
#if defined(WINDOWS) || defined(WINDOWS64)
int inet_pton(int af, const char* src, char* dst);
#endif
int inet_pton4(const char* src, char* dst);

#endif
