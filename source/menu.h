#ifndef RETRODS_MENU_H
#define RETRODS_MENU_H

#include <nds.h>

// Logical menu items. The grid renders them in this order.
typedef enum {
    MENU_DS = 0,
    MENU_GB,
    MENU_GBC,
    MENU_GBA,
    MENU_NES,
    MENU_SNES,
    MENU_RECENT,
    MENU_SETTINGS,
    MENU_COUNT
} MenuId;

typedef struct {
    const char *name;        // shown under the icon on the bottom screen
    const char *shortLabel;  // small label inside the card (e.g. "DS", "GBA")
    const char *description; // one-line description on the top screen
} MenuItem;

extern const MenuItem g_menu[MENU_COUNT];

// Grid is 4 columns x 2 rows.
#define MENU_COLS 4
#define MENU_ROWS 2

// Pixel geometry of a single icon card on the bottom screen.
#define MENU_GRID_TOP    24
#define MENU_GRID_BOTTOM 176
#define MENU_CELL_W      (256 / MENU_COLS)
#define MENU_CELL_H      ((MENU_GRID_BOTTOM - MENU_GRID_TOP) / MENU_ROWS)
#define MENU_CARD_PAD_X  6
#define MENU_CARD_PAD_Y  6
#define MENU_CARD_W      (MENU_CELL_W - 2 * MENU_CARD_PAD_X)
#define MENU_CARD_H      (MENU_CELL_H - 2 * MENU_CARD_PAD_Y)

// Returns the top-left pixel of the card at (col, row) on the bottom screen.
void menuCardOrigin(int col, int row, int *x, int *y);

// Returns true if (tx, ty) hits a card, writing its index into *out.
bool menuHitTest(int tx, int ty, int *out);

// Rendering. Pass framebuffers and the currently selected index.
void menuDrawBottom(u16 *fb, int selected, bool pressed);
void menuDrawTop(u16 *fb, int selected);

#endif
