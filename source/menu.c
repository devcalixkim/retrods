#include "menu.h"
#include "ui.h"

// ---------------------------------------------------------------------------
// Items
// ---------------------------------------------------------------------------

const MenuItem g_menu[MENU_COUNT] = {
    { "DS Game",          "DS",   "Launch a Nintendo DS cartridge image."   },
    { "Game Boy",         "GB",   "Play original Game Boy titles."          },
    { "Game Boy Color",   "GBC",  "Play Game Boy Color titles."             },
    { "Game Boy Advance", "GBA",  "Play Game Boy Advance titles."           },
    { "NES",              "NES",  "Play Nintendo Entertainment System ROMs."},
    { "SNES",             "SNES", "Play Super NES ROMs."                    },
    { "Recent",           "REC",  "Resume what you played last."            },
    { "Settings",         "SET",  "Theme, sound, region, about."            },
};

// ---------------------------------------------------------------------------
// Grid geometry helpers
// ---------------------------------------------------------------------------

void menuCardOrigin(int col, int row, int *x, int *y) {
    *x = col * MENU_CELL_W + MENU_CARD_PAD_X;
    *y = MENU_GRID_TOP + row * MENU_CELL_H + MENU_CARD_PAD_Y;
}

bool menuHitTest(int tx, int ty, int *out) {
    if (ty < MENU_GRID_TOP || ty >= MENU_GRID_BOTTOM) return false;
    int col = tx / MENU_CELL_W;
    int row = (ty - MENU_GRID_TOP) / MENU_CELL_H;
    if (col < 0 || col >= MENU_COLS) return false;
    if (row < 0 || row >= MENU_ROWS) return false;
    int idx = row * MENU_COLS + col;
    if (idx >= MENU_COUNT) return false;

    // Confirm the touch falls inside the card rect, not the gutter.
    int cx, cy;
    menuCardOrigin(col, row, &cx, &cy);
    if (tx < cx || tx >= cx + MENU_CARD_W) return false;
    if (ty < cy || ty >= cy + MENU_CARD_H) return false;

    *out = idx;
    return true;
}

// ---------------------------------------------------------------------------
// Abstract glyphs. Each takes a side length `s` so we can reuse the same
// drawing routines for the small card icon (32) and the larger preview on
// the top screen (56). We deliberately avoid evoking any official platform
// logo — these are simple silhouettes hinting at form factor.
// ---------------------------------------------------------------------------

static void glyphDS(u16 *fb, int cx, int cy, int s, u16 color) {
    int w = s * 3 / 4;
    int h = s * 7 / 16;
    int r = (s >= 48) ? 3 : 2;
    uiRoundedRect(fb, cx - w / 2, cy - h - 1, w, h, r, color);
    uiRoundedRect(fb, cx - w / 2, cy + 1,     w, h, r, color);
}

static void glyphGB(u16 *fb, int cx, int cy, int s, u16 color) {
    int w = s / 2 + 2;
    int h = s * 7 / 8;
    int sw = w - 6;
    int sh = h / 3;
    int r = (s >= 48) ? 4 : 3;
    int x = cx - w / 2;
    int y = cy - h / 2;
    uiRoundedRect(fb, x, y, w, h, r, color);
    // screen well in the upper third
    uiRect(fb, x + 3, y + 3, sw, sh, color);
}

static void glyphGBC(u16 *fb, int cx, int cy, int s, u16 color) {
    // Same silhouette as GB plus a small power dot for differentiation.
    glyphGB(fb, cx, cy, s, color);
    int w = s / 2 + 2;
    int h = s * 7 / 8;
    int x = cx - w / 2;
    int y = cy - h / 2;
    int sh = h / 3;
    uiFillRect(fb, x + w - 6, y + 3 + sh + 2, 2, 2, color);
}

static void glyphGBA(u16 *fb, int cx, int cy, int s, u16 color) {
    int w = s * 7 / 8;
    int h = s / 2;
    int r = (s >= 48) ? 4 : 3;
    int x = cx - w / 2;
    int y = cy - h / 2;
    uiRoundedRect(fb, x, y, w, h, r, color);
    // centered screen well
    int sw = w / 2;
    int sh = h - 6;
    uiRect(fb, cx - sw / 2, y + 3, sw, sh, color);
}

static void glyphNES(u16 *fb, int cx, int cy, int s, u16 color) {
    int w = s * 7 / 8;
    int h = s / 2;
    int r = 1;
    int x = cx - w / 2;
    int y = cy - h / 2;
    uiRoundedRect(fb, x, y, w, h, r, color);
    // cartridge label band
    uiHLine(fb, x + 2, y + h / 3, w - 4, color);
}

static void glyphSNES(u16 *fb, int cx, int cy, int s, u16 color) {
    int w = s * 7 / 8;
    int h = s / 2;
    int r = (s >= 48) ? 3 : 2;
    int x = cx - w / 2;
    int y = cy - h / 2;
    uiRoundedRect(fb, x, y, w, h, r, color);
    // two label bands (suggests the two-tone SNES cart)
    uiHLine(fb, x + 2, y + h / 3,         w - 4, color);
    uiHLine(fb, x + 2, y + 2 * h / 3,     w - 4, color);
}

static void glyphRecent(u16 *fb, int cx, int cy, int s, u16 color) {
    // A clock-face suggestion: rounded square + two perpendicular hands.
    int w = s * 5 / 8;
    int r = (s >= 48) ? 5 : 4;
    int x = cx - w / 2;
    int y = cy - w / 2;
    uiRoundedRect(fb, x, y, w, w, r, color);
    // hour + minute hands
    uiHLine(fb, cx, cy, w / 3, color);
    uiVLine(fb, cx, cy - w / 3, w / 3, color);
}

static void glyphSettings(u16 *fb, int cx, int cy, int s, u16 color) {
    // Three slider bars, each with a different "knob" position.
    int w = s * 3 / 4;
    int gap = (s >= 48) ? 8 : 5;
    int x = cx - w / 2;
    int y = cy - gap;
    for (int i = 0; i < 3; ++i) {
        int row = y + i * gap;
        uiHLine(fb, x, row, w, color);
        // knob — a small filled rectangle along each track
        int knob = (i == 0 ? 4 : (i == 1 ? w - 8 : w / 2 - 2));
        uiFillRect(fb, x + knob, row - 1, 4, 3, color);
    }
}

typedef void (*GlyphFn)(u16 *, int, int, int, u16);
static const GlyphFn kGlyphs[MENU_COUNT] = {
    glyphDS, glyphGB, glyphGBC, glyphGBA,
    glyphNES, glyphSNES, glyphRecent, glyphSettings,
};

// ---------------------------------------------------------------------------
// Bottom screen — icon grid
// ---------------------------------------------------------------------------

static void drawTitleBar(u16 *fb, const char *title) {
    uiText(fb, 8, 4, title, UI_C_GRAY);
    uiHLine(fb, 0, 19, UI_SCREEN_W, UI_C_HAIRLINE);
}

static void drawHintBar(u16 *fb, const char *hint) {
    uiHLine(fb, 0, UI_SCREEN_H - 16, UI_SCREEN_W, UI_C_HAIRLINE);
    uiText(fb, 8, UI_SCREEN_H - 12, hint, UI_C_GRAY);
}

static void drawCard(u16 *fb, int idx, bool selected, int dy) {
    int col = idx % MENU_COLS;
    int row = idx / MENU_COLS;
    int x, y;
    menuCardOrigin(col, row, &x, &y);

    // Card body
    if (selected) {
        uiFillRoundedRect(fb, x, y, MENU_CARD_W, MENU_CARD_H, 4, UI_C_PANEL);
        uiRoundedRect    (fb, x, y, MENU_CARD_W, MENU_CARD_H, 4, UI_C_ACCENT);
    } else {
        uiFillRoundedRect(fb, x, y, MENU_CARD_W, MENU_CARD_H, 4, UI_C_WHITE);
        uiRoundedRect    (fb, x, y, MENU_CARD_W, MENU_CARD_H, 4, UI_C_HAIRLINE);
    }

    // Glyph centered in the upper area of the card, shifted by dy when pressed.
    int gx = x + MENU_CARD_W / 2;
    int gy = y + MENU_CARD_H / 2 - 6 + dy;
    kGlyphs[idx](fb, gx, gy, 32, UI_C_DARK);

    // Short label centered near the bottom of the card.
    uiTextCentered(fb, x + MENU_CARD_W / 2, y + MENU_CARD_H - 12 + dy,
                   g_menu[idx].shortLabel, UI_C_DARK);
}

void menuDrawBottom(u16 *fb, int selected, bool pressed) {
    uiClear(fb, UI_C_WHITE);
    drawTitleBar(fb, "RetroDS");

    for (int i = 0; i < MENU_COUNT; ++i) {
        bool sel = (i == selected);
        int dy = (sel && pressed) ? 1 : 0;
        drawCard(fb, i, sel, dy);
    }

    drawHintBar(fb, "A: Open   B: Back   Tap: Select");
}

// ---------------------------------------------------------------------------
// Top screen — selected item info panel
// ---------------------------------------------------------------------------

void menuDrawTop(u16 *fb, int selected) {
    uiClear(fb, UI_C_WHITE);

    // Title strip
    uiText(fb, 8, 4, "RetroDS  v0.1", UI_C_GRAY);
    uiHLine(fb, 0, 19, UI_SCREEN_W, UI_C_HAIRLINE);

    if (selected < 0 || selected >= MENU_COUNT) return;
    const MenuItem *m = &g_menu[selected];

    // Large glyph in the upper-middle area.
    kGlyphs[selected](fb, UI_SCREEN_W / 2, 64, 56, UI_C_DARK);

    // Soft accent underline under the glyph to lift it visually.
    uiHLine(fb, UI_SCREEN_W / 2 - 30, 100, 60, UI_C_HAIRLINE);

    // Name (large-ish — our font is 8px so we just print it bold-ish by
    // doubling the spacing visually with extra centered text).
    uiTextCentered(fb, UI_SCREEN_W / 2, 112, m->name, UI_C_DARK);

    // One-line description in gray.
    uiTextCentered(fb, UI_SCREEN_W / 2, 128, m->description, UI_C_GRAY);

    // Bottom hint bar
    uiHLine(fb, 0, UI_SCREEN_H - 16, UI_SCREEN_W, UI_C_HAIRLINE);
    uiText(fb, 8,   UI_SCREEN_H - 12, "A: Open",  UI_C_GRAY);
    uiText(fb, 200, UI_SCREEN_H - 12, "B: Back",  UI_C_GRAY);
}
