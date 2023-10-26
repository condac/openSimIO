#ifndef __vectorfont_h__
#define __vectorfont_h__

typedef struct {
    float x;
    float y;
} CoordRec, *CoordPtr;

typedef struct {
    int num_coords;
    const CoordRec* coord;
} StrokeRec, *StrokePtr;

typedef struct {
    int num_strokes;
    const StrokeRec* stroke;
    float center;
    float right;
} StrokeCharRec, *StrokeCharPtr;

typedef struct {
    const char* name;
    int num_chars;
    const StrokeCharRec* ch;
    float top;
    float bottom;
} StrokeFontRec, *StrokeFontPtr;

void vectorFontCharacter2(int c);
void drawLineText(const char* text, float x, float y, float size, int align);
float textHeight(float size);
#endif /* __vectorfont_h__ */
