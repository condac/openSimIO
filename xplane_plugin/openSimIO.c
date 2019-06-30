#include "openSimIO.h"

// Downloaded from https://developer.x-plane.com/code-sample/hello-world-sdk-3/


#include "XPLMDisplay.h"
#include "XPLMGraphics.h"
#include <string.h>
#if IBM
	#include <windows.h>
#endif
#if LIN
	#include <GL/gl.h>
#elif __GNUC__
	#include <OpenGL/gl.h>
#else
	#include <GL/gl.h>
#endif

#ifndef XPLM300
	#error This is made to be compiled against the XPLM300 SDK
#endif

// An opaque handle to the window we will create
static XPLMWindowID	g_window;

// Callbacks we will register when we create our window
void				draw_hello_world(XPLMWindowID in_window_id, void * in_refcon);
int					dummy_mouse_handler(XPLMWindowID in_window_id, int x, int y, int is_down, void * in_refcon) { return 0; }
XPLMCursorStatus	dummy_cursor_status_handler(XPLMWindowID in_window_id, int x, int y, void * in_refcon) { return xplm_CursorDefault; }
int					dummy_wheel_handler(XPLMWindowID in_window_id, int x, int y, int wheel, int clicks, void * in_refcon) { return 0; }
void				dummy_key_handler(XPLMWindowID in_window_id, char key, XPLMKeyFlags flags, char virtual_key, void * in_refcon, int losing_focus) { }

static float	MyFlightLoopCallback(float inElapsedSinceLastCall,float inElapsedTimeSinceLastFlightLoop, int inCounter, void* inRefcon);

static XPLMDataRef		gDataRef = NULL;
static XPLMDataRef		testDataRef = NULL;

int TeensyControls_show=0;
int cport_nr=16;        /* /dev/ttyS0 (COM1 on windows) */
int bdrate=115200;       /* 9600 baud */

int slaveId = 0;




PLUGIN_API int XPluginStart(
							char *		outName,
							char *		outSig,
							char *		outDesc)
{
	strcpy(outName, "openSimIO");
	strcpy(outSig, "github.condac.openSimIO");
	strcpy(outDesc, "A plug-in for openSimIO.");



	XPLMCreateWindow_t params;
	params.structSize = sizeof(params);
	params.visible = 1;
	params.drawWindowFunc = draw_hello_world;
	// Note on "dummy" handlers:
	// Even if we don't want to handle these events, we have to register a "do-nothing" callback for them
	params.handleMouseClickFunc = dummy_mouse_handler;
	params.handleRightClickFunc = dummy_mouse_handler;
	params.handleMouseWheelFunc = dummy_wheel_handler;
	params.handleKeyFunc = dummy_key_handler;
	params.handleCursorFunc = dummy_cursor_status_handler;
	params.refcon = NULL;
	params.layer = xplm_WindowLayerFloatingWindows;
	// Opt-in to styling our window like an X-Plane 11 native window
	// If you're on XPLM300, not XPLM301, swap this enum for the literal value 1.
  #ifndef XPLM301
	  params.decorateAsFloatingWindow = 1;
  #else
    params.decorateAsFloatingWindow = xplm_WindowDecorationRoundRectangle;
  #endif
	// Set the window's initial bounds
	// Note that we're not guaranteed that the main monitor's lower left is at (0, 0)...
	// We'll need to query for the global desktop bounds!
	int left, bottom, right, top;
	XPLMGetScreenBoundsGlobal(&left, &top, &right, &bottom);
	params.left = left + 50;
	params.bottom = bottom + 150;
	params.right = params.left + 200;
	params.top = params.bottom + 200;

	g_window = XPLMCreateWindowEx(&params);

	// Position the window as a "free" floating window, which the user can drag around
	XPLMSetWindowPositioningMode(g_window, xplm_WindowPositionFree, -1);
	// Limit resizing our window: maintain a minimum width/height of 100 boxels and a max width/height of 300 boxels
	XPLMSetWindowResizingLimits(g_window, 200, 200, 300, 300);
	XPLMSetWindowTitle(g_window, "openSimIO");

  // Menu
  XPLMMenuID	myMenu;
	int			mySubMenuItem;

  /* First we put a new menu item into the plugin menu.
	 * This menu item will contain a submenu for us. */
	mySubMenuItem = XPLMAppendMenuItem(
						XPLMFindPluginsMenu(),	/* Put in plugins menu */
						"openSimIO",				/* Item Title */
						0,						/* Item Ref */
						1);						/* Force English */

	/* Now create a submenu attached to our menu item. */
	myMenu = XPLMCreateMenu(
						"openSimIO",
						XPLMFindPluginsMenu(),
						mySubMenuItem, 			/* Menu Item to attach to. */
						MyMenuHandlerCallback,	/* The handler */
						0);						/* Handler Ref */

	/* Append a few menu items to our submenu.  We will use the refcon to
	 * store the amount we want to change the radio by. */
	XPLMAppendMenuItem(
						myMenu,
						"Decrement Nav1",
						(void *) 3,
						1);
	XPLMAppendMenuItem(
						myMenu,
						"Increment Nav1",
						(void *) 2,
						1);
  XPLMAppendMenuItem(
						myMenu,
						"Toggle debug",
						(void *) 1,
						1);

	/* Look up our data ref.  You find the string name of the data ref
	 * in the master list of data refs, including in HTML form in the
	 * plugin SDK.  In this case, we want the nav1 frequency. */
	gDataRef = XPLMFindDataRef("sim/cockpit/switches/pitot_heat_on");
  testDataRef = XPLMFindDataRef("sim/joystick/yoke_roll_ratio");

  TeensyControls_show = 1;
  TeensyControls_display_init();
  display("started openSimIO");


  readConfig();

  char mode[]={'8','N','1',0};


  if(RS232_OpenComport(cport_nr, bdrate, mode, 0))  {
    display("Can not open comport\n");

    //return(0);
  }

  /* Register our callback for once a second.  Positive intervals
	 * are in seconds, negative are the negative of sim frames.  Zero
	 * registers but does not schedule a callback for time. */
	XPLMRegisterFlightLoopCallback(
			MyFlightLoopCallback,	/* Callback */
			1.0,					/* Interval */
			NULL);					/* refcon not used. */
	return g_window != NULL;
}

PLUGIN_API void	XPluginStop(void)
{
	// Since we created the window, we'll be good citizens and clean it up
	XPLMDestroyWindow(g_window);
	g_window = NULL;
}

PLUGIN_API void XPluginDisable(void) { }
PLUGIN_API int  XPluginEnable(void)  { return 1; }
PLUGIN_API void XPluginReceiveMessage(XPLMPluginID inFrom, int inMsg, void * inParam) { }

void	draw_hello_world(XPLMWindowID in_window_id, void * in_refcon)
{
	// Mandatory: We *must* set the OpenGL state before drawing
	// (we can't make any assumptions about it)
	XPLMSetGraphicsState(
						 0 /* no fog */,
						 0 /* 0 texture units */,
						 0 /* no lighting */,
						 0 /* no alpha testing */,
						 1 /* do alpha blend */,
						 1 /* do depth testing */,
						 0 /* no depth writing */
						 );

	int l, t, r, b;
	XPLMGetWindowGeometry(in_window_id, &l, &t, &r, &b);

	float col_white[] = {1.0, 1.0, 1.0}; // red, green, blue

	XPLMDrawString(col_white, l + 10, t - 20, "Hello world!", NULL, xplmFont_Proportional);
  XPLMDrawString(col_white, l + 10, t - 20, "Hello world2!", NULL, xplmFont_Proportional);
}



void	MyMenuHandlerCallback(void* inMenuRef, void* inItemRef) {
  display("%d",(int)inItemRef);
  if((int) inItemRef == 1) {
    // toggle console
    TeensyControls_show = !TeensyControls_show;
    TeensyControls_display_toggle();
  }
  if((int) inItemRef == 2) {
    // set pitot
    if (gDataRef != NULL)	{
  		/* We read the data ref, add the increment and set it again.
  		 * This changes the nav frequency. */
  		XPLMSetDatai(gDataRef,1);
  	}
  }
  if((int) inItemRef == 3) {
    // set pitot
    if (gDataRef != NULL)	{
  		/* We read the data ref, add the increment and set it again.
  		 * This changes the nav frequency. */
  		XPLMSetDatai(gDataRef,0);
  	}
  }
	/* This is our handler for the menu item.  Our inItemRef is the refcon
	 * we registered in our XPLMAppendMenuItem calls.  It is either +1000 or
	 * -1000 depending on which menu item is picked. */
	//if (gDataRef != NULL)	{
		/* We read the data ref, add the increment and set it again.
		 * This changes the nav frequency. */
		//XPLMSetDatai(gDataRef,(int) inItemRef);
	//}



}

void parseInputPin(char* data) {
  char* digital = strstr(data, "D"); // this also removes leading spaces
  char* analog = strstr(data, "A");

  char* pinName = "";

  if(digital != NULL) {
    //display("Found Digital %s", digital);
    //int start = strstr(data+digital, " ");
    int var = 0;
    char* slask = strstr(digital, " ");;
    sscanf (slask, "%d", &var);
    char seps[] = " ";
    pinName = strtok(digital, seps);
    //display("value %d %s", var, pinName);

    if (gDataRef != NULL)	{
  		XPLMSetDatai(gDataRef,var);
  	}
  }
  if(analog != NULL) {
    display("Found Analog %s", analog);
    int var = 0;
    //sscanf (analog, "%s %d", pinName, &var);

    char* slask = strstr(analog, " ");
    sscanf (slask, "%d", &var);
    char seps[] = " ";
    pinName = strtok(analog, seps);
    display("value %d , %s", var, pinName);
    float ftemp = var * 1.0;
    setAnalogData(1, 0, pinName, (float)ftemp);

    /*if (testDataRef != NULL)	{
      float temp = ((float)var/512.0) - 1.0;
  		XPLMSetDataf(testDataRef,temp);
  	}*/

  }
}

void parseToken(char* data) {
  // data is sepperated with comma
  char seps[] = ",";
  char* token;
  int test = 1;

  token = strtok(data, seps);

  while (token != NULL)  {
    display("tokens %s %d", token, test);
    char* tempString = malloc(300);
    char* tmp = tempString;
    strcpy(tempString, token);
    parseInputPin(tempString);
    free(tmp);
    token = strtok(NULL, seps);
    test++;
  }
}

void parseMessage(char* data) {
  // data is sepperated with semicolon
  char seps[] = ";";
  char* token;
  int test = 1;
  
  token = strtok(data, seps);
  if (token != NULL) {
    //display("first token %s", token);
  }
  token = strtok(NULL, seps);
  if (token != NULL) {
    //display("second token %s", token);
    sscanf (token, "%d", &slaveId);
  }
  token = strtok(NULL, seps);
  if (token != NULL) {
    parseToken(token);
  }

}

int getSlaveId(char* data) {
  // data is sepperated with semicolon
  char seps[] = ";";
  char* token;
  int id = 0;
  token = strtok(data, seps);
  if (token != NULL) {
    token = strtok(NULL, seps);
    if (token != NULL) {
      sscanf (token, "%d", &id);
    }
  }
  return id;
}


void parseSerialInput( char* data, int len) {

  char seps[] = "{}\n";
  char* token;
  int test = 1;
  //char* slask;
  //strncpy(slask, data, len);

  token = strtok(data, seps);
  while (token != NULL)  {
    token = strstr(token, "99;");
    if(token != NULL) {
      display("data for me %s", data);

      //strncpy(slask, data, len);
      //slaveId = getSlaveId(slask);
      parseMessage(data);

      //display("id for me %d %s",id,  data);
    }
    token = strtok(NULL, seps);
    test++;
  }
}


float	MyFlightLoopCallback( float inElapsedSinceLastCall,
           float  inElapsedTimeSinceLastFlightLoop,
           int    inCounter,
           void*  inRefcon)
{
	/* The actual callback.  First we read the sim's time and the data. */
	float	elapsed = XPLMGetElapsedTime();

  int i, n;
  char buf[4096];

  n = RS232_PollComport(cport_nr, buf, 4095);

  if(n > 0)    {
    buf[n] = '\0';   /* always put a "null" at the end of a string! */
    parseSerialInput(buf, n);
    /*for(i=0; i < n; i++)      {
      if(buf[i] < 32) { // replace unreadable control-codes by dots

        buf[i] = '.';
      }
    }*/

    //display("received %i bytes: %s\n", n, (char *)buf);
  }


	/* Write the data to a file. */
	// display("Time=%f.\n",elapsed);

	/* Return 1.0 to indicate that we want to be called again in 1 second. */
	return 0.02;
}
