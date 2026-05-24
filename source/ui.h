#ifndef RETRODS_UI_H
#define RETRODS_UI_H

#include <nds.h>

#define UI_SCREEN_W 256
#define UI_SCREEN_H 192
#define UI_STRIDE   256

// ---------------------------------------------------------------------------
// Color palette — all values include the mandatory BIT(15) alpha bit.
//
// The palette is tuned to mimic the clean white-on-white DS Lite aesthetic:
// white background, very light gray separators, muted blue selection, and
// just enough shadow depth to read the card hierarchy.
// ---------------------------------------------------------------------------
#define UI_C_WHITE       (RGB15(31,31,31) | BIT(15))
#define UI_C_OFFWHITE    (RGB15(30,30,30) | BIT(15))
#define UI_C_BGBOT       (RGB15(28,28,29) | BIT(15)) // gradient bottom
#define UI_C_HAIRLINE    (RGB15(22,22,23) | BIT(15))
#define UI_C_SHADOW      (RGB15(19,19,20) | BIT(15))
#define UI_C_GRAY        (RGB15(16,16,17) | BIT(15)) // secondary text
#define UI_C_DARK        (RGB15(4, 4, 5)  | BIT(15)) // primary text
#define UI_C_ACCENT      (RGB15(8, 16,27) | BIT(15)) // selection border
#define UI_C_ACCENT_SOFT (RGB15(20,26,31) | BIT(15)) // selection fill tint

// Platform card background colors — saturated enough to read, dark enough
// that white glyphs have adequate contrast.
#define UI_C_CAT_DS   (RGB15(6, 14,26) | BIT(15)) // deep sky blue
#define UI_C_CAT_GB   (RGB15(14,14,12) | BIT(15)) // warm mid-gray
#define UI_C_CAT_GBC  (RGB15(14, 6,24) | BIT(15)) // dark purple
#define UI_C_CAT_GBA  (RGB15(8,  8,24) | BIT(15)) // dark indigo
#define UI_C_CAT_NES  (RGB15(24, 6, 6) | BIT(15)) // dark red
#define UI_C_CAT_SNES (RGB15(10, 4,24) | BIT(15)) // deep violet
#define UI_C_CAT_REC  (RGB15(24,18, 4) | BIT(15)) // amber / gold
#define UI_C_CAT_SET  (RGB15(12,12,13) | BIT(15)) // neutral charcoal

// ---------------------------------------------------------------------------
// Primitives
// ---------------------------------------------------------------------------
void uiClear(u16 *fb, u16 color);
void uiGradientBg(u16 *fb, u16 ctop, u16 cbot);
void uiPixel(u16 *fb, int x, int y, u16 color);
void uiHLine(u16 *fb, int x, int y, int w, u16 color);
void uiVLine(u16 *fb, int x, int y, int h, u16 color);
void uiRect(u16 *fb, int x, int y, int w, int h, u16 color);
void uiFillRect(u16 *fb, int x, int y, int w, int h, u16 color);
void uiRoundedRect(u16 *fb, int x, int y, int w, int h, int r, u16 color);
void uiFillRoundedRect(u16 *fb, int x, int y, int w, int h, int r, u16 color);

// Card drop shadow: filled rounded rect offset (d, d) pixels.
void uiShadow(u16 *fb, int x, int y, int w, int h, int r, int d, u16 color);

// ---------------------------------------------------------------------------
// Text — standard 8×8 and 2× upscaled (16×16 per glyph)
// ---------------------------------------------------------------------------
void uiChar(u16 *fb, int x, int y, char c, u16 color);
void uiText(u16 *fb, int x, int y, const char *s, u16 color);
int  uiTextWidth(const char *s);
void uiTextCentered(u16 *fb, int cx, int y, const char *s, u16 color);

void uiText2x(u16 *fb, int x, int y, const char *s, u16 color);
int  uiText2xWidth(const char *s);
void uiText2xCentered(u16 *fb, int cx, int y, const char *s, u16 color);

#endif
