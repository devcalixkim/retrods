#include "ui.h"
#include "font.h"
#include <string.h>

// ---------------------------------------------------------------------------
// Corner offset table: kCornerOffsets[r][row] = x-inset at that corner row.
// ---------------------------------------------------------------------------
static const unsigned char kCornerOffsets[7][7] = {
    { 0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF },
    { 0,    0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF },
    { 1,    0,    0, 0xFF, 0xFF, 0xFF, 0xFF },
    { 2,    1,    0,    0, 0xFF, 0xFF, 0xFF },
    { 2,    1,    0,    0,    0, 0xFF, 0xFF },
    { 3,    1,    1,    0,    0,    0, 0xFF },
    { 3,    2,    1,    1,    0,    0,    0 },
};

// ---------------------------------------------------------------------------
// Background fills
// ---------------------------------------------------------------------------

void uiClear(u16 *fb, u16 color) {
    for (int y = 0; y < UI_SCREEN_H; ++y) {
        u16 *row = fb + y * UI_STRIDE;
        for (int x = 0; x < UI_SCREEN_W; ++x) row[x] = color;
    }
}

void uiGradientBg(u16 *fb, u16 ctop, u16 cbot) {
    int rt = ctop & 0x1F, gt = (ctop >> 5) & 0x1F, bt = (ctop >> 10) & 0x1F;
    int rb = cbot & 0x1F, gb = (cbot >> 5) & 0x1F, bb = (cbot >> 10) & 0x1F;
    int H  = UI_SCREEN_H - 1;
    for (int y = 0; y < UI_SCREEN_H; ++y) {
        int r = rt + (rb - rt) * y / H;
        int g = gt + (gb - gt) * y / H;
        int b = bt + (bb - bt) * y / H;
        u16 col = (u16)(r | (g << 5) | (b << 10) | BIT(15));
        u16 *row = fb + y * UI_STRIDE;
        for (int x = 0; x < UI_SCREEN_W; ++x) row[x] = col;
    }
}

// ---------------------------------------------------------------------------
// Low-level primitives
// ---------------------------------------------------------------------------

void uiPixel(u16 *fb, int x, int y, u16 color) {
    if ((unsigned)x >= UI_SCREEN_W || (unsigned)y >= UI_SCREEN_H) return;
    fb[y * UI_STRIDE + x] = color;
}

void uiHLine(u16 *fb, int x, int y, int w, u16 color) {
    if ((unsigned)y >= UI_SCREEN_H) return;
    if (x < 0) { w += x; x = 0; }
    if (x + w > UI_SCREEN_W) w = UI_SCREEN_W - x;
    if (w <= 0) return;
    u16 *p = fb + y * UI_STRIDE + x;
    for (int i = 0; i < w; ++i) p[i] = color;
}

void uiVLine(u16 *fb, int x, int y, int h, u16 color) {
    if ((unsigned)x >= UI_SCREEN_W) return;
    if (y < 0) { h += y; y = 0; }
    if (y + h > UI_SCREEN_H) h = UI_SCREEN_H - y;
    if (h <= 0) return;
    u16 *p = fb + y * UI_STRIDE + x;
    for (int i = 0; i < h; ++i) p[i * UI_STRIDE] = color;
}

void uiRect(u16 *fb, int x, int y, int w, int h, u16 color) {
    uiHLine(fb, x,         y,         w, color);
    uiHLine(fb, x,         y + h - 1, w, color);
    uiVLine(fb, x,         y,         h, color);
    uiVLine(fb, x + w - 1, y,         h, color);
}

void uiFillRect(u16 *fb, int x, int y, int w, int h, u16 color) {
    for (int j = 0; j < h; ++j) uiHLine(fb, x, y + j, w, color);
}

void uiFillRoundedRect(u16 *fb, int x, int y, int w, int h, int r, u16 color) {
    if (r < 0) r = 0;
    if (r > 6) r = 6;
    if (r * 2 > w) r = w / 2;
    if (r * 2 > h) r = h / 2;
    for (int j = 0; j < h; ++j) {
        int inset = 0;
        if (j < r) inset = kCornerOffsets[r][j];
        else if (j >= h - r) inset = kCornerOffsets[r][h - 1 - j];
        uiHLine(fb, x + inset, y + j, w - 2 * inset, color);
    }
}

void uiRoundedRect(u16 *fb, int x, int y, int w, int h, int r, u16 color) {
    if (r < 0) r = 0;
    if (r > 6) r = 6;
    if (r * 2 > w) r = w / 2;
    if (r * 2 > h) r = h / 2;
    uiHLine(fb, x + r, y,         w - 2 * r, color);
    uiHLine(fb, x + r, y + h - 1, w - 2 * r, color);
    uiVLine(fb, x,         y + r, h - 2 * r, color);
    uiVLine(fb, x + w - 1, y + r, h - 2 * r, color);
    // Corner arcs. Each row plots a horizontal run from this row's inset up
    // to the previous row's inset, bridging any step so there are no gaps.
    for (int j = 0; j < r; ++j) {
        int inset = kCornerOffsets[r][j];
        int prev  = (j > 0) ? kCornerOffsets[r][j - 1] : r;
        int end   = (prev > inset) ? prev : inset + 1;
        for (int k = inset; k < end; ++k) {
            uiPixel(fb, x + k,           y + j,         color);
            uiPixel(fb, x + w - 1 - k,   y + j,         color);
            uiPixel(fb, x + k,           y + h - 1 - j, color);
            uiPixel(fb, x + w - 1 - k,   y + h - 1 - j, color);
        }
    }
}

void uiShadow(u16 *fb, int x, int y, int w, int h, int r, int d, u16 color) {
    uiFillRoundedRect(fb, x + d, y + d, w, h, r, color);
}

// ---------------------------------------------------------------------------
// Text — 8×8 standard
// ---------------------------------------------------------------------------

void uiChar(u16 *fb, int x, int y, char c, u16 color) {
    unsigned ch = (unsigned char)c;
    if (ch < 0x20 || ch > 0x7F) ch = 0x20;
    const unsigned char *glyph = font8x8[ch - 0x20];
    for (int row = 0; row < 8; ++row) {
        unsigned bits = glyph[row];
        for (int col = 0; col < 8; ++col)
            if (bits & (1u << col)) uiPixel(fb, x + col, y + row, color);
    }
}

void uiText(u16 *fb, int x, int y, const char *s, u16 color) {
    int cx = x;
    while (*s) {
        if (*s == '\n') { y += 10; cx = x; ++s; continue; }
        uiChar(fb, cx, y, *s, color);
        cx += 8;
        ++s;
    }
}

int uiTextWidth(const char *s) {
    int n = 0;
    while (*s) { if (*s != '\n') ++n; ++s; }
    return n * 8;
}

void uiTextCentered(u16 *fb, int cx, int y, const char *s, u16 color) {
    uiText(fb, cx - uiTextWidth(s) / 2, y, s, color);
}

// ---------------------------------------------------------------------------
// Text — 2× upscaled (each pixel becomes a 2×2 block)
// ---------------------------------------------------------------------------

void uiText2x(u16 *fb, int x, int y, const char *s, u16 color) {
    int cx = x;
    while (*s) {
        if (*s == '\n') { y += 18; cx = x; ++s; continue; }
        unsigned ch = (unsigned char)*s;
        if (ch < 0x20 || ch > 0x7F) ch = 0x20;
        const unsigned char *glyph = font8x8[ch - 0x20];
        for (int row = 0; row < 8; ++row) {
            unsigned bits = glyph[row];
            for (int col = 0; col < 8; ++col) {
                if (bits & (1u << col)) {
                    uiPixel(fb, cx + col * 2,     y + row * 2,     color);
                    uiPixel(fb, cx + col * 2 + 1, y + row * 2,     color);
                    uiPixel(fb, cx + col * 2,     y + row * 2 + 1, color);
                    uiPixel(fb, cx + col * 2 + 1, y + row * 2 + 1, color);
                }
            }
        }
        cx += 16;
        ++s;
    }
}

int uiText2xWidth(const char *s) {
    return uiTextWidth(s) * 2;
}

void uiText2xCentered(u16 *fb, int cx, int y, const char *s, u16 color) {
    uiText2x(fb, cx - uiText2xWidth(s) / 2, y, s, color);
}
