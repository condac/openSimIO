#include "openSimIO.h"
#include "statusDisplay.h"

// this is borrowed from https://github.com/PaulStoffregen/X-Plane_Plugin

#define WIDTH 890
#define LINES 60

static XPLMWindowID wStatus = NULL;

static char textStatus[LINES * 256];
int headStatus = 0;

#define EQ(s1, s2) (strncmp(s1, s2, strlen(s2)) == 0)

void drawStatus(XPLMWindowID inWindowID, void* inRefcon) {
    int i, n, left, top, right, bottom, y;
    char* str;
    float yellow[] = {1.0, 1.0, 0.5};
    float red[] = {1.0, 0.4, 0.4};
    float lime[] = {0.8, 1.0, 0.6};
    float aqua[] = {0.4, 0.5, 0.8};
    float white[] = {1.0, 1.0, 1.0};
    float* color;
    int lineSpacing = 16;

    XPLMGetWindowGeometry(inWindowID, &left, &top, &right, &bottom);
    XPLMDrawTranslucentDarkBox(left, top, right, bottom);
    y = bottom + 6;
    n = headStatus;
    for (i = 0; i < LINES; i++) {
        color = white;
        str = textStatus + n * 256;
        if (EQ(str, "Write"))
            color = yellow;
        if (EQ(str, "Error"))
            color = red;
        if (EQ(str, "Command"))
            color = lime;
        if (EQ(str, "Update"))
            color = aqua;

        XPLMDrawString(color, left + 5, y, str, NULL, xplmFont_Basic);
        if (n == 0)
            n = LINES;
        n--;
        y += lineSpacing;
    }
}

void keyStatus(XPLMWindowID inWindowID, char inKey, XPLMKeyFlags inFlags, char inVirtualKey, void* inRefcon, int losingFocus) {
}

int mouseStatus(XPLMWindowID inWindowID, int x, int y, XPLMMouseStatus inMouse, void* inRefcon) {
    return 0;
}

void statusDisplayInit(void) {
    memset(textStatus, 0, sizeof(textStatus));
    printf("TeensyControls_display_init\n");
    if (statusDisplayShow) {
        if (!wStatus) {
            wStatus = XPLMCreateWindow(50, 1440, 950, 50, 1, drawStatus, keyStatus, mouseStatus, NULL);
            headStatus = 0;
        }
    } else {
        if (wStatus) {
            XPLMDestroyWindow(wStatus);
            wStatus = NULL;
        }
    }
}

void statusDisplayToggle(void) {
    printf("TeensyControls_display_init\n");
    if (statusDisplayShow) {
        if (!wStatus) {
            wStatus = XPLMCreateWindow(50, 1440, 950, 50, 1, drawStatus, keyStatus, mouseStatus, NULL);
            headStatus = 0;
            //memset(text, 0, sizeof(text));
        }
    } else {
        if (wStatus) {
            XPLMDestroyWindow(wStatus);
            wStatus = NULL;
        }
    }
}

void statusClear() {
    memset(textStatus, 0, sizeof(textStatus));
}
int statusPrintf(const char* format, ...) {
    va_list args;
    int len;

    va_start(args, format);
    if (++headStatus >= LINES)
        headStatus = 0;
    len = vsnprintf(textStatus + (headStatus * 256), 255, format, args);
    if (len <= 0 || len > 255)
        return 0;
    // TODO: use XPLMMeasureString to trim string
    //XPLMMeasureString(xplmFont_Basic, "test", 4);

    return len;
}
