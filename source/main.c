// RetroDS — DS Lite homebrew launcher (v0.1)
//
// v0.1 scope: dual-screen init, DS Lite-flavored icon grid on the touch
// screen, info preview on the top screen, D-pad + A/B + touch handling.
// No ROM scanning, no boxart, no emulator handoff yet.

#include <nds.h>
#include <stdio.h>

#include "ui.h"
#include "menu.h"
#include "input.h"

static u16 *s_fbMain;  // bottom (touch) screen framebuffer
static u16 *s_fbSub;   // top screen framebuffer

static void initVideo(void) {
    // Swap displays so the more capable "main" engine drives the touch
    // screen, where we need full bitmap drawing for the icon grid.
    lcdMainOnBottom();

    powerOn(POWER_ALL_2D);

    // Both screens use mode 5, which lets BG3 act as a 16-bit bitmap layer.
    videoSetMode   (MODE_5_2D);
    videoSetModeSub(MODE_5_2D);

    // VRAM bank assignments. BG_BMP16 256x256 is exactly 128KB, which fills
    // a single bank — so one bank per screen.
    vramSetBankA(VRAM_A_MAIN_BG);   // bottom (main engine)
    vramSetBankC(VRAM_C_SUB_BG);    // top    (sub engine)

    int bgMain = bgInit   (3, BgType_Bmp16, BgSize_B16_256x256, 0, 0);
    int bgSub  = bgInitSub(3, BgType_Bmp16, BgSize_B16_256x256, 0, 0);

    s_fbMain = bgGetGfxPtr(bgMain);
    s_fbSub  = bgGetGfxPtr(bgSub);

    // Solid white background while we wait for the first real frame.
    uiClear(s_fbMain, UI_C_WHITE);
    uiClear(s_fbSub,  UI_C_WHITE);
}

int main(void) {
    initVideo();

    // Touch input is driven by libnds defaults; no extra init required.

    InputState st;
    inputInit(&st, 0);

    // After activation we hold the "pressed" visual for a few frames so the
    // user gets clear feedback that A or a tap registered, even though v0.1
    // doesn't actually launch anything yet.
    int activationHold = 0;

    while (1) {
        swiWaitForVBlank();
        scanKeys();
        inputStep(&st);

        if (st.activated && activationHold == 0) {
            activationHold = 6;
        }
        if (activationHold > 0) {
            // Force the pressed visual regardless of touch state.
            if (!st.pressedVisual) { st.pressedVisual = true; st.dirty = true; }
            --activationHold;
            if (activationHold == 0) st.dirty = true;  // redraw un-pressed
        }

        // B is a no-op at the root menu in v0.1 (nothing to back out to).
        // We still consume it so the field doesn't linger.
        (void)st.back;

        if (st.dirty) {
            menuDrawBottom(s_fbMain, st.selection, st.pressedVisual);
            menuDrawTop   (s_fbSub,  st.selection);
            st.dirty = false;
        }
    }

    return 0;
}
