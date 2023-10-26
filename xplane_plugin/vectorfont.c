

#include "openSimIO.h"
#include "vectorfont.h"
#include "jasroman.font"


float line_text_scale = 1.0;

float text_scale = 1.0;

float scalefactor = 0.2*0.781818181818;

void vectorFontCharacter(int c) {
    const StrokeCharRec* ch;
    const StrokeRec* stroke;
    const CoordRec* coord;
    StrokeFontPtr fontinfo = (StrokeFontPtr)&vectorFontMonoRoman;
    int i, j;

    if (c < 0 || c >= fontinfo->num_chars)
        return;
    ch = &(fontinfo->ch[c]);
    if (ch) {
        for (i = ch->num_strokes, stroke = ch->stroke; i > 0; i--, stroke++) {
            glLineWidth(line_text_scale);
            glBegin(GL_LINE_STRIP);
            for (j = stroke->num_coords, coord = stroke->coord; j > 0; j--, coord++) {
                glVertex2f(coord->x, coord->y);
            }
            glEnd();
        }
        glTranslatef(ch->right, 0.0, 0.0);
    }
}

float vectorFontWidth(int c) {
    StrokeFontPtr fontinfo = (StrokeFontPtr)&vectorFontMonoRoman;
    const StrokeCharRec* ch;

    if (c < 0 || c >= fontinfo->num_chars)
        return 0;
    ch = &(fontinfo->ch[c]);
    if (ch)
        return ch->right;
    else
        return 0;
}
float vectorFontLength(const char* string) {
    int c;
    float length;
    StrokeFontPtr fontinfo = (StrokeFontPtr)&vectorFontMonoRoman;
    const StrokeCharRec* ch;
    int i = 0;

    length = 0;

    while (string[i] != '\0') {
        c = string[i];
        if (c >= 0 && c < fontinfo->num_chars) {
            ch = &(fontinfo->ch[c]);
            if (ch) {
                length += ch->right;
            }
        }
        i++;
    }
    return length;
}

float textHeight(float size) {
    return 1.0 * vectorFontWidth('W') * size * text_scale * scalefactor;
}

void drawLineText(const char* text, float x, float y, float size, int align) {
    //int width = vectorFontWidth('W');
    float length = vectorFontLength(text);
    float halign = 0.0;
    float valign = 0.0;
    int i = 0;

    if (align == 1) {
        halign = -length / 2;
    }
    if (align == 2) {
        halign = -length;
    }

    glPushMatrix();
    glTranslatef(x, y, 0.0);
    glScalef(scalefactor * size * text_scale, scalefactor * size * text_scale, 0.0);
    glTranslatef(halign, valign, 0.0);
    while (text[i] != '\0') {
        vectorFontCharacter(text[i]);
        i++;
    }
    glPopMatrix();
}