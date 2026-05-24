#include "sound.h"
#include <nds.h>

static int s_chan      = -1;
static int s_countdown = 0;

void soundInit(void) {
    soundEnable();
}

static void play(int hz, int frames) {
    if (s_chan >= 0) { soundKill(s_chan); s_chan = -1; }
    s_chan      = soundPlayPSG(DutyCycle_50, (u16)hz, 72, 127);
    s_countdown = frames;
}

void soundPlayNav(void)    { play(440, 3); }  // ~50 ms soft click
void soundPlaySelect(void) { play(880, 4); }  // ~67 ms confirm tone

void soundStep(void) {
    if (s_countdown > 0 && --s_countdown == 0 && s_chan >= 0) {
        soundKill(s_chan);
        s_chan = -1;
    }
}
