
#include "openSimIO.h"
#include "udp.h"
#include "pins.h"
#include "config.h"
#include "rs232.h"
#include "statusDisplay.h"
// Downloaded from https://developer.x-plane.com/code-sample/hello-world-sdk-3/


#include "XPLMDisplay.h"
#include "XPLMGraphics.h"
#include <string.h>
#if IBM
#   include <windows.h>
#endif
#if LIN
#   include <GL/gl.h>           // apt install mesa-common-dev
#elif __GNUC__
        //#include <OpenGL/gl.h>
#else
#   include <GL/gl.h>
#endif

//#ifndef XPLM300
//      #error This is made to be compiled against the XPLM300 SDK
//#endif

// An opaque handle to the window we will create

#ifdef XPLM301
static XPLMWindowID g_window;

// Callbacks we will register when we create our window
void draw_hello_world(XPLMWindowID in_window_id, void *in_refcon);
int dummy_mouse_handler(XPLMWindowID in_window_id, int x, int y, int is_down, void *in_refcon) {
   return 0;
}
XPLMCursorStatus dummy_cursor_status_handler(XPLMWindowID in_window_id, int x, int y, void *in_refcon) {
   return xplm_CursorDefault;
}
int dummy_wheel_handler(XPLMWindowID in_window_id, int x, int y, int wheel, int clicks, void *in_refcon) {
   return 0;
}
void dummy_key_handler(XPLMWindowID in_window_id, char key, XPLMKeyFlags flags, char virtual_key, void *in_refcon,
                       int losing_focus) {
}
#endif
static float MyFlightLoopCallback(float inElapsedSinceLastCall, float inElapsedTimeSinceLastFlightLoop, int inCounter,
                                  void *inRefcon);


static XPLMDataRef gDataRef = NULL;
static XPLMDataRef testDataRef = NULL;

int TeensyControls_show = 0;
int statusDisplayShow = 0;

int slaveId = 0;
float lastSignal = 0.0;

int useEthernet = 0;
int useSerial = 0;

master_struct masters[MAXMASTERS];

pin_struct *pins;

int nrOfLines = 0;
int nrOfPins = 0;

void reloadConfig() {
   XPLMDebugString("openSimIO:reloadConfig\n");
   readConfig();

   sendConfigReset();
}

PLUGIN_API int XPluginStart(char *outName, char *outSig, char *outDesc) {
   strcpy(outName, "openSimIO");
   strcpy(outSig, "github.condac.openSimIO");
   strcpy(outDesc, "A plug-in for openSimIO.");


#ifdef XPLM301
   XPLMCreateWindow_t params;
   params.structSize = sizeof(params);
   params.visible = 1;
   params.drawWindowFunc = draw_hello_world;
   // Note on "dummy" handlers:
   // Even if we don't want to handle these events, we have to register a "do-nothing" callback for them
   params.handleMouseClickFunc = dummy_mouse_handler;
   //
   params.handleMouseWheelFunc = dummy_wheel_handler;
   params.handleKeyFunc = dummy_key_handler;
   params.handleCursorFunc = dummy_cursor_status_handler;
   params.refcon = NULL;
   //
   // Opt-in to styling our window like an X-Plane 11 native window
   // If you're on XPLM300, not XPLM301, swap this enum for the literal value 1.

   params.handleRightClickFunc = dummy_mouse_handler;

   params.layer = xplm_WindowLayerFloatingWindows;
   params.decorateAsFloatingWindow = xplm_WindowDecorationRoundRectangle;

   // Set the window's initial bounds
   // Note that we're not guaranteed that the main monitor's lower left is at (0, 0)...
   // We'll need to query for the global desktop bounds!
   int left, bottom, right, top;
   XPLMGetScreenBoundsGlobal(&left, &top, &right, &bottom);
   params.left = left + 50;
   params.bottom = bottom + 150;
   params.right = params.left + 200;
   params.top = params.bottom + 200;

   //g_window = XPLMCreateWindowEx(&params);
#endif
   // Position the window as a "free" floating window, which the user can drag around
   //XPLMSetWindowPositioningMode(g_window, xplm_WindowPositionFree, -1);
   // Limit resizing our window: maintain a minimum width/height of 100 boxels and a max width/height of 300 boxels
   //XPLMSetWindowResizingLimits(g_window, 200, 200, 300, 300);
   //XPLMSetWindowTitle(g_window, "openSimIO");

   // Menu
   XPLMMenuID myMenu;
   int mySubMenuItem;

   /* First we put a new menu item into the plugin menu.
    * This menu item will contain a submenu for us. */
   mySubMenuItem = XPLMAppendMenuItem(XPLMFindPluginsMenu(),    /* Put in plugins menu */
                                      "openSimIO",      /* Item Title */
                                      0,        /* Item Ref */
                                      1);       /* Force English */

   /* Now create a submenu attached to our menu item. */
   myMenu = XPLMCreateMenu("openSimIO", XPLMFindPluginsMenu(), mySubMenuItem,   /* Menu Item to attach to. */
                           MyMenuHandlerCallback,       /* The handler */
                           0);  /* Handler Ref */

   /* Append a few menu items to our submenu.  We will use the refcon to
    * store the amount we want to change the radio by. */

   XPLMAppendMenuItem(myMenu, "Toggle debug", (void *)1, 1);
   XPLMAppendMenuItem(myMenu, "Show input status", (void *)5, 1);
   XPLMAppendMenuItem(myMenu, "Send config", (void *)4, 1);
   /* Look up our data ref.  You find the string name of the data ref
    * in the master list of data refs, including in HTML form in the
    * plugin SDK.  In this case, we want the nav1 frequency. */
   gDataRef = XPLMFindDataRef("sim/cockpit/switches/pitot_heat_on");
   testDataRef = XPLMFindDataRef("sim/joystick/yoke_roll_ratio");

   TeensyControls_show = 0;

   TeensyControls_display_init();
   display("started openSimIO");

   statusDisplayInit();

   XPLMDebugString("openSimIO:read config\n");

   reloadConfig();
   XPLMDebugString("openSimIO:read config done\n");


   //asock = createUDPSocket("192.168.0.105", 34555);
   /* Register our callback for once a second.  Positive intervals
    * are in seconds, negative are the negative of sim frames.  Zero
    * registers but does not schedule a callback for time. */
   XPLMRegisterFlightLoopCallback(MyFlightLoopCallback, /* Callback */
                                  1.0,  /* Interval */
                                  NULL);        /* refcon not used. */
   return 1;                    //g_window != NULL;
}

PLUGIN_API void XPluginStop(void) {
   // Since we created the window, we'll be good citizens and clean it up

#ifdef XPLM301
   XPLMDestroyWindow(g_window);
   g_window = NULL;
#endif
}

PLUGIN_API void XPluginDisable(void) {
}
PLUGIN_API int XPluginEnable(void) {
   return 1;
}
PLUGIN_API void XPluginReceiveMessage(XPLMPluginID inFrom, int inMsg, void *inParam) {
}

void draw_hello_world(XPLMWindowID in_window_id, void *in_refcon) {
   // Mandatory: We *must* set the OpenGL state before drawing
   // (we can't make any assumptions about it)
   XPLMSetGraphicsState(0 /* no fog */ ,
                        0 /* 0 texture units */ ,
                        0 /* no lighting */ ,
                        0 /* no alpha testing */ ,
                        1 /* do alpha blend */ ,
                        1 /* do depth testing */ ,
                        0       /* no depth writing */
      );

   int l, t, r, b;
   XPLMGetWindowGeometry(in_window_id, &l, &t, &r, &b);

   float col_white[] = { 1.0, 1.0, 1.0 };       // red, green, blue

   XPLMDrawString(col_white, l + 10, t - 20, "Hello world!", NULL, xplmFont_Proportional);
   XPLMDrawString(col_white, l + 10, t - 20, "Hello world2!", NULL, xplmFont_Proportional);
}



void MyMenuHandlerCallback(void *inMenuRef, void *inItemRef) {
   //display("%d",(int)inItemRef);
   int incommand = (int) inItemRef;
   if (incommand == 1) {
      // toggle console
      TeensyControls_show = !TeensyControls_show;
      TeensyControls_display_toggle();
   }
   if (incommand == 2) {
      // set pitot
      if (gDataRef != NULL) {
         /* We read the data ref, add the increment and set it again.
          * This changes the nav frequency. */
         XPLMSetDatai(gDataRef, 1);
      }
   }
   if (incommand == 3) {
      // set pitot
      if (gDataRef != NULL) {
         /* We read the data ref, add the increment and set it again.
          * This changes the nav frequency. */
         XPLMSetDatai(gDataRef, 0);
      }
   }
   if (incommand == 4) {
      reloadConfig();
      sendConfigReset();
   }
   if (incommand == 5) {
      if (statusDisplayShow == 0) {
         statusDisplayShow = 1;
      } else {
         statusDisplayShow = 0;
      }
      statusDisplayToggle();
   }
   /* This is our handler for the menu item.  Our inItemRef is the refcon
    * we registered in our XPLMAppendMenuItem calls.  It is either +1000 or
    * -1000 depending on which menu item is picked. */
   //if (gDataRef != NULL) {
   /* We read the data ref, add the increment and set it again.
    * This changes the nav frequency. */
   //XPLMSetDatai(gDataRef,(int) inItemRef);
   //}



}




void parseToken(char *data) {
   // data is sepperated with comma

}

void parseMessage(char *data) {
   // data is sepperated with semicolon



   int masterId;
   char inputString[4000];
   //display("parseMessage data %s", data);
   sscanf(data, "{99;%d;%d;%4000[^;];", &masterId, &slaveId, inputString);
   //display("parseMessage master:%d slave:%d string: %s", masterId, slaveId, inputString);
   //parseToken(inputString);


   char x;
   int pos = 0;


   x = inputString[pos];
   while (x != '\0') {

      char *tempString = malloc(300);
      char *tmp = tempString;
      strcpy(tempString, inputString + pos);
      for (int i = 0; i < 10; i++) {
         if (tempString[i] == ',') {
            pos = pos + i;
            tempString[i] = '\0';
            //display("parseMessage found substring: %s",tempString);
         }
      }
      parseInputPin(tempString, masterId, slaveId);     // in pins.c
      free(tmp);
      pos++;
      x = inputString[pos];
   }

}

int getSlaveId(char *data) {
   // data is sepperated with semicolon
   char seps[] = ";";
   char *token;
   int id = 0;
   token = strtok(data, seps);
   if (token != NULL) {
      token = strtok(NULL, seps);
      if (token != NULL) {
         sscanf(token, "%d", &id);
      }
   }
   return id;
}


void parseSerialInput(char *data, int len) {

   //display("parseSerialInput %s %d",data,  len);
   char seps[] = "{}\n";
   char *token;
   int test = 1;
   //char* slask;
   //strncpy(slask, data, len);

   token = strtok(data, seps);
   while (token != NULL) {
      char *token2 = strstr(token, "99;");
      if (token2 != NULL) {
         //display("data for me %s", data);

         //strncpy(slask, data, len);
         //slaveId = getSlaveId(slask);
         parseMessage(data);

         //display("id for me %d %s",id,  data);
      }
      token2 = strstr(token, "98;");
      if (token2 != NULL) {
         XPLMDebugString("reconfigure 98\n");
         display("reconfigure %s", data);

         //strncpy(slask, data, len);
         //slaveId = getSlaveId(slask);
         //parseMessage(data);
         if (lastSignal > 5.0) {
            reloadConfig();
         }
         //display("id for me %d %s",id,  data);
      }
      token = strtok(NULL, seps);
      test++;
   }
}


int ifCharInArray(char *str, char val) {
   char x;
   int pos = 0;
   x = str[pos];
   while (x != '\0') {
      if (str[pos] == val) {
         return pos;
      }
      pos++;
      x = str[pos];
   }
   return -1;
}

char inputbuf[8200];



float MyFlightLoopCallback(float inElapsedSinceLastCall, float inElapsedTimeSinceLastFlightLoop, int inCounter,
                           void *inRefcon) {

   /* The actual callback.  First we read the sim's time and the data. */
   XPLMDebugString("openSimIO: flightloop\n");

   float elapsed = XPLMGetElapsedTime();
   setTimeStep(elapsed);

   // call function too check if there is an ongoing event for pushed buttons
   setStepLoop();

   int n;
   char buf[4096];

   for (int i = 0; i < MAXMASTERS; i++) {
      if (masters[i].type == IS_SERIAL) {
         n = RS232_PollComport(masters[i].portNumber, buf, 4095);

         if (n > 0) {

            buf[n] = '\0';      // always put a "null" at the end of a string!
            if (ifCharInArray(buf, '}') == -1) {
               // ONly half of message recieved or garbage
               //display("received %i bytes: %s\n", n, (char *)buf);
            }
            parseSerialInput(buf, n);

         }
      }
      if (masters[i].type == IS_ETH) {

         while (ifMessage(masters[i].socket)) {
            int res = readUDP(masters[i].socket, buf, 4095);
            //int test = ifMessage(asock);
            if (res > 0) {
               lastSignal = elapsed;
               buf[res] = '\0';
               if (ifCharInArray(buf, '}') == -1) {
                  // ONly half of message recieved or garbage
                  //display("received %i bytes: %s\n", n, (char *)buf);
               } else {
                  //display("received udp %i bytes: %s\n", res, (char *)buf);
                  parseSerialInput(buf, res);
               }
            }
         }

         if (lastSignal < elapsed - 5.0) {
            display("Error! no connection for 5s");
            lastSignal = elapsed;
            sendConfigReset();
         }

      }
   }
   //sendConfig();
   handleOutputs();

   if (statusDisplayShow == 1) {
      drawStatusDisplayInfo();
   }

   /* Return 1.0 to indicate that we want to be called again in 1 second. */
   return 0.01;
}
