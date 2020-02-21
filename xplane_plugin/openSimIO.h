#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>

#define USE_PRINTF_DEBUG
//#define PRINTF_ADDR "10.0.0.123"
#define PRINTF_ADDR "127.0.0.1"

#if defined(MACOSX)
#   define APL 1
#   define IBM 0
#   define LIN 0
#   include <unistd.h>
#   include <sys/socket.h>
#   include <netinet/in.h>
#   include <arpa/inet.h>
#   include <sys/select.h>
#   include <pthread.h>
#   include <time.h>
#   include <sys/time.h>
#   include <IOKit/IOKitLib.h>
#   include <IOKit/hid/IOHIDLib.h>
#   include <IOKit/hid/IOHIDDevice.h>
#elif defined(LINUX)
#   define APL 0
#   define IBM 0
#   define LIN 1
#   include <unistd.h>
#   include <sys/socket.h>
#   include <sys/types.h>
#   include <sys/stat.h>
#   include <sys/ioctl.h>
#   include <netinet/in.h>
#   include <arpa/inet.h>
#   include <sys/select.h>
#   include <pthread.h>
#   include <time.h>
#   ifdef USE_PRINTF_DEBUG
#      include <execinfo.h>
#   endif
#   include <linux/hidraw.h>
#elif defined(WINDOWS) || defined(WINDOWS64)
#   define APL 0
#   define IBM 1
#   define LIN 0
#   ifndef WINVER
#      define WINVER 0x0500
#   endif
#   ifndef _WIN32_WINNT
#      define _WIN32_WINNT 0x0500
#   endif
//#include <winsock2.h>
#   include <windows.h>
//#include <ws2tcpip.h>
//#include <winuser.h>
#   include <setupapi.h>
#   include <hidsdi.h>
#   include <dbt.h>
//#include "thread.h"
#endif

/*
XPLM301—X-Plane 11.20 & newer (64-bit only)
XPLM300—X-Plane 11.10 & newer (64-bit only)
XPLM210—X-Plane 10.00 & newer (10.20 required for 64-bit plugins)
XPLM200—X-Plane 9.00 & newer
*/
#define XPLM200 1
//#define XPLM210 1
//#define XPLM300 1
//#define XPLM301 1

#include "XPLMDisplay.h"        // download xplane SDK and put in SDK folder https://developer.x-plane.com/sdk/plugin-sdk-downloads/
#include "XPLMGraphics.h"
#include "XPLMUtilities.h"
#include "XPLMDataAccess.h"
#include "XPLMProcessing.h"
#include "XPLMMenus.h"

#   define IS_ETH 1
#   define IS_SERIAL 2
#   define MAXMASTERS 10


#include "udp.h"
typedef struct {

   int nr;
   int type;
   int udpPort;
   char ip[18];
   char serialport[32];
   udpSocket socket;
   int portNumber;

} master_struct;

void MyMenuHandlerCallback(void *inMenuRef, void *inItemRef);
void reloadConfig();

extern master_struct masters[MAXMASTERS];
#include "pins.h"
extern pin_struct *pins;

extern int nrOfLines;
extern int nrOfPins;
// display.c
extern int TeensyControls_show;
#define display(...) (TeensyControls_show ? TeensyControls_display(__VA_ARGS__) : 0)
void TeensyControls_display_init(void);
void TeensyControls_display_toggle(void);
int TeensyControls_display(const char *format, ...) __attribute__ ((format(printf, 1, 2)));;
int TeensyControls_display(const char *format, ...);
