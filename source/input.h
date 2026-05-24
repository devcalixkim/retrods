#ifndef RETRODS_INPUT_H
#define RETRODS_INPUT_H

#include <nds.h>

typedef struct {
    int  selection;       // current selection index (0..MENU_COUNT-1)
    bool pressedVisual;   // render the selection in "pressed" state
    bool activated;       // user triggered "open" this frame (A or touch up)
    bool back;            // user triggered "back" this frame (B)
    bool dirty;           // something visual changed; main loop should redraw
} InputState;

// Initialize input state. `startSel` is the default selection (usually 0).
void inputInit(InputState *st, int startSel);

// Poll input for one frame; updates st. Call once per frame after scanKeys().
void inputStep(InputState *st);

#endif
