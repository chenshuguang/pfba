//
// Created by cpasjuste on 21/11/16.
//

#ifndef _RENDERER_H_
#define _RENDERER_H_

#include <cstdint>
#include <cstring>
#include <cstdarg>
#include <cstdio>

#include "font.h"
#include "texture.h"
#include "rect.h"
#include "color.h"
#include "utility.h"

#ifndef MAX_PATH
#define MAX_PATH 512
#endif

class Renderer {

public:

    Renderer(int w, int h);
    virtual ~Renderer();

    virtual Font *LoadFont(const char *path, int size) {return NULL;};  // to implement
    virtual void DrawFont(Font *font, int x, int y, const char *fmt, ...) {};// to implement, no truncate
    virtual void DrawFont(Font *font, Rect *dst, Color *color, const char *fmt, ...);
    virtual void DrawFont(Font *font, Rect *dst, Color *color, bool centerX, bool centerY, const char *fmt, ...);

    virtual Texture *CreateTexture(int w, int h) {return NULL;};  // to implement
    virtual Texture *LoadTexture(const char *file) {return NULL;};  // to implement
    virtual void DrawTexture(Texture *texture, int x, int y, int w, int h, float rotation) {}; // to implement
    virtual void DrawTexture(Texture *texture, int x, int y, int w, int h);
    virtual void DrawTexture(Texture *texture, int x, int y);
    virtual Rect DrawTexture(Texture *texture, const Rect *rect, bool fit = true);
    virtual int LockTexture(Texture * texture, const Rect *rect, void **pixels, int *pitch) {return 0;};  // to implement
    virtual void UnlockTexture(Texture * texture) {};  // to implement

    virtual void SetShader(int shader) {};  // to implement

    virtual void DrawLine(int x1, int y1, int x2, int y2);
    virtual void DrawLine(int x1, int y1, int x2, int y2, Color *color) {};  // to implement

    virtual void DrawRect(Rect *rect, Color *color, bool fill = true) {};  // to implement
    virtual void DrawRect(int x, int y, int w, int h, uint8_t r, uint8_t g, uint8_t b, uint8_t a, bool fill = true);
    virtual void DrawBorder(Rect *rect, Color *color, bool inside = true);

    virtual void Clip(Rect *rect) {};  // to implement

    virtual Rect GetWindowSize() {return Rect{0, 0, 0, 0};};  // to implement

    virtual void ClearScreen() {};  // to implement
    virtual void FlipScreen() {};  // to implement

    virtual void Delay(unsigned int ms) {};  // to implement

    Color *color;
    int shaderCount = 0;
    int shaderCurrent = 0;
};

#ifdef __PSP2__
#include "psp2_renderer.h"
#else
#include "sdl2_renderer.h"
#endif

#endif //_RENDERER_H_