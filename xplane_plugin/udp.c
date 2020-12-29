#include "openSimIO.h"
#include <stdio.h>
#include <string.h>
#include "iotypes.h"
#include "udp.h"



#define NS_INADDRSZ  4
#define NS_IN6ADDRSZ 16
#define NS_INT16SZ   2

#if defined(WINDOWS) || defined(WINDOWS64)
int inet_pton(int af, const char *src, char *dst) {
   switch (af) {
   case AF_INET:
      return inet_pton4(src, dst);
      //case AF_INET6:
      //    return inet_pton6(src, dst);
   default:
      return -1;
   }
}
#endif

int inet_pton4(const char *src, char *dst) {
   uint8_t tmp[NS_INADDRSZ], *tp;

   int saw_digit = 0;
   int octets = 0;
   *(tp = tmp) = 0;

   int ch;
   while ((ch = *src++) != '\0') {
      if (ch >= '0' && ch <= '9') {
         uint32_t n = *tp * 10 + (ch - '0');

         if (saw_digit && *tp == 0)
            return 0;

         if (n > 255)
            return 0;

         *tp = n;
         if (!saw_digit) {
            if (++octets > 4)
               return 0;
            saw_digit = 1;
         }
      } else if (ch == '.' && saw_digit) {
         if (octets == 4)
            return 0;
         *++tp = 0;
         saw_digit = 0;
      } else
         return 0;
   }
   if (octets < 4)
      return 0;

   memcpy(dst, tmp, NS_INADDRSZ);

   return 1;
}

/*
int inet_pton6(const char *src, char *dst)
{
    static const char xdigits[] = "0123456789abcdef";
    uint8_t tmp[NS_IN6ADDRSZ];

    uint8_t *tp = (uint8_t*) memset(tmp, '\0', NS_IN6ADDRSZ);
    uint8_t *endp = tp + NS_IN6ADDRSZ;
    uint8_t *colonp = NULL;


    if (*src == ':')
    {
        if (*++src != ':')
            return 0;
    }

    const char *curtok = src;
    int saw_xdigit = 0;
    uint32_t val = 0;
    int ch;
    while ((ch = tolower(*src++)) != '\0')
    {
        const char *pch = strchr(xdigits, ch);
        if (pch != NULL)
        {
            val <<= 4;
            val |= (pch - xdigits);
            if (val > 0xffff)
                return 0;
            saw_xdigit = 1;
            continue;
        }
        if (ch == ':')
        {
            curtok = src;
            if (!saw_xdigit)
            {
                if (colonp)
                    return 0;
                colonp = tp;
                continue;
            }
            else if (*src == '\0')
            {
                return 0;
            }
            if (tp + NS_INT16SZ > endp)
                return 0;
            *tp++ = (uint8_t) (val >> 8) & 0xff;
            *tp++ = (uint8_t) val & 0xff;
            saw_xdigit = 0;
            val = 0;
            continue;
        }
        if (ch == '.' && ((tp + NS_INADDRSZ) <= endp) &&
                inet_pton4(curtok, (char*) tp) > 0)
        {
            tp += NS_INADDRSZ;
            saw_xdigit = 0;
            break; // '\0' was seen by inet_pton4().
        }
        return 0;
    }
    if (saw_xdigit)
    {
        if (tp + NS_INT16SZ > endp)
            return 0;
        *tp++ = (uint8_t) (val >> 8) & 0xff;
        *tp++ = (uint8_t) val & 0xff;
    }
    if (colonp != NULL)
    {

        const int n = tp - colonp;

        if (tp == endp)
            return 0;

        for (int i = 1; i <= n; i++)
        {
            endp[-i] = colonp[n - i];
            colonp[n - i] = 0;
        }
        tp = endp;
    }
    if (tp != endp)
        return 0;

    memcpy(dst, tmp, NS_IN6ADDRSZ);

    return 1;
}
*/


int setnonblocking(udpSocket sock) {
#if defined(WINDOWS) || defined(WINDOWS64)
   unsigned long mode = 1;
   ioctlsocket(sock.sock, FIONBIO, &mode);
   return 1;
#else
   int i;
   i = fcntl(sock.sock, F_GETFL);
   if (fcntl(sock.sock, F_SETFL, i | O_NONBLOCK) < 0)
      return 0;
   return 1;
#endif
}

udpSocket createUDPSocket(char *ipIn, int portIn) {

   udpSocket sock;

   // save ip
   strncpy(sock.ip, ipIn, 17);
   // save port
   sock.port = portIn+100;           // portIn;

   display("createUDPSocket %s %d", sock.ip, sock.port);


   sock.sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

   if (sock.sock == -1) {
      display("Error failed to create socket");
      return sock;
   }
   // bind to port
   struct sockaddr_in recvaddr;
   recvaddr.sin_family = AF_INET;
   recvaddr.sin_addr.s_addr = INADDR_ANY;
   recvaddr.sin_port = htons(sock.port);

   if (bind(sock.sock, (struct sockaddr *)&recvaddr, sizeof(recvaddr)) == -1) {

      display("Error failed to bind socket");
      return sock;
   }
/*
	// Set timeout
  #ifdef IBM2
	DWORD timeout = 100;
  #else
	struct timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 1;
  #endif
*/
   /*if (setsockopt(sock.sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout)) < 0) {
      display("Error failed to set timeout");
      } */
   setnonblocking(sock);

   sock.port = portIn;

   return sock;
}

#if defined(WINDOWS) || defined(WINDOWS64)

#else
int dataAvaible(udpSocket sock) {
   //return 1;
   fd_set readfds;
   fcntl(sock.sock, F_SETFL, O_NONBLOCK);
   struct timeval tv;

   FD_ZERO(&readfds);
   FD_SET(sock.sock, &readfds);

   tv.tv_sec = 0;
   tv.tv_usec = 0;

   int res = select(sock.sock + 1, &readfds, NULL, NULL, &tv);


   return res;
}
#endif
int sendUDP(udpSocket socket, char buffer[], int len) {

   struct sockaddr_in dest;

   dest.sin_family = AF_INET;
   dest.sin_port = htons(socket.port);

   inet_pton(AF_INET, socket.ip, &dest.sin_addr.s_addr);

   if (len <= 0) {
      //display("sendUDP len0 %s %d %s %d", socket.ip, socket.port, buffer, len);
      return -1;
   }
   //display("sendUDP %s %d %s %d", socket.ip, socket.port, buffer, len);
   int res = sendto(socket.sock, buffer, len, 0, (const struct sockaddr *)&dest, sizeof(dest));

   if (res < 0) {
      return -1;
   }

   if (res < len) {
      // we did not manage to send all data.
   }
   return res;
}

/*int readUDP2(udpSocket sock, char buffer[], int len) {

  // create non blocking udp read
  int res = 0;
  //char buf[4098];
  struct sockaddr_in remote;
  int slen = sizeof(remote);
  if (dataAvaible(sock) > 0) {
    res = recvfrom(sock.sock, buffer, len, 0, (struct sockaddr*) &remote, &slen);
    if (res>0) {
      buffer[res] = '\0';
      //display("Received %d from %s:%d %s\n\n", res, inet_ntoa(remote.sin_addr), ntohs(remote.sin_port), buffer);
      return res;
    }
    else {
      return -1;
    }
  }
}*/


int readUDP(udpSocket sock, char buffer[], int len) {
#if defined(WINDOWSzzzzz) || defined(WINDOWSzzzz64)

   // zzzzz* Turns out windows do not need this when compiling through mingw in linux

   // Windows readUDP needs the select command- minimum timeout is 1ms.
   // Without this playback becomes choppy

   // Definitions
   FD_SET stReadFDS;
   FD_SET stExceptFDS;

   // Setup for Select
   FD_ZERO(&stReadFDS);
   FD_SET(sock.sock, &stReadFDS);
   FD_ZERO(&stExceptFDS);
   FD_SET(sock.sock, &stExceptFDS);

   struct timeval tv;
   tv.tv_sec = 0;
   tv.tv_usec = 100000;

   // Select Command
   int status = select(-1, &stReadFDS, (FD_SET *) 0, &stExceptFDS, &tv);
   if (status < 0) {
      display("Select command error");
      return -1;
   }
   if (status == 0) {
      // No data
      return 0;
   }
   status = recv(sock.sock, buffer, len, 0);
#else
   // For apple or linux-just read - will timeout in 0.5 ms
   int status = (int)recv(sock.sock, buffer, len, 0);
#endif
   if (status < 0) {
      //display("Error reading socket");
   }
   return status;
}


int ifMessage(udpSocket sock) {
   fd_set rfd;
   FD_ZERO(&rfd);
   FD_SET(sock.sock, &rfd);
   struct timeval timeout;
   timeout.tv_sec = 0;
   timeout.tv_usec = 0;
   int ret = select(sock.sock + 1, &rfd, NULL, NULL, &timeout);
   return ret;
}

void closeSocket(udpSocket sock) {
   close(sock.sock);
}
