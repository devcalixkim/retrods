#include "input.h"
#include "menu.h"

// Internal: which card index is currently being touched, or -1.
// We track this across frames so that a touch *release* over the same card
// counts as an activation, matching DS Lite's tap-to-open feel.
static int s_touchOriginCard = -1;

void inputInit(InputState *st, int startSel) {
    st->selection     = startSel;
    st->pressedVisual = false;
    st->activated     = false;
    st->back          = false;
    st->dirty         = true;  // force first draw
    s_touchOriginCard = -1;
}

static void moveSelection(InputState *st, int dCol, int dRow) {
    int col = st->selection % MENU_COLS;
    int row = st->selection / MENU_COLS;
    col = (col + dCol + MENU_COLS) % MENU_COLS;
    row = (row + dRow + MENU_ROWS) % MENU_ROWS;
    int next = row * MENU_COLS + col;
    if (next >= MENU_COUNT) {
        // Slot would be past the populated range; collapse back to the last
        // valid index. Currently MENU_COUNT == MENU_COLS * MENU_ROWS so this
        // is dead code, but it keeps the layout flexible.
        next = MENU_COUNT - 1;
    }
    if (next != st->selection) {
        st->selection = next;
        st->dirty = true;
    }
}

void inputStep(InputState *st) {
    // Reset per-frame events.
    st->activated = false;
    st->back = false;

    uint32 down = keysDown();
    uint32 held = keysHeld();
    uint32 up   = keysUp();

    // D-pad — edge triggered so a tap moves once, not at 60Hz.
    if (down & KEY_LEFT)  moveSelection(st, -1,  0);
    if (down & KEY_RIGHT) moveSelection(st, +1,  0);
    if (down & KEY_UP)    moveSelection(st,  0, -1);
    if (down & KEY_DOWN)  moveSelection(st,  0, +1);

    // A / B
    if (down & KEY_A) { st->activated = true; }
    if (down & KEY_B) { st->back      = true; }

    // Touch: tap-to-open. Press on a card sets it as the origin and selects
    // it; releasing over the same card activates it. Dragging off cancels.
    bool prevPressed = st->pressedVisual;
    bool nowPressed = false;

    if (held & KEY_TOUCH) {
        touchPosition tp;
        touchRead(&tp);
        int hit;
        if (menuHitTest(tp.px, tp.py, &hit)) {
            if (s_touchOriginCard == -1) {
                // First frame of this touch — anchor it to this card.
                s_touchOriginCard = hit;
            }
            if (hit == s_touchOriginCard) {
                if (st->selection != hit) {
                    st->selection = hit;
                    st->dirty = true;
                }
                nowPressed = true;
            }
        }
    } else if (up & KEY_TOUCH) {
        // Stylus left the screen this frame. If it lifted off the same card
        // it pressed down on, count it as an activation.
        if (s_touchOriginCard != -1 && prevPressed) {
            st->activated = true;
        }
        s_touchOriginCard = -1;
    } else {
        s_touchOriginCard = -1;
    }

    if (nowPressed != prevPressed) {
        st->pressedVisual = nowPressed;
        st->dirty = true;
    }
}
