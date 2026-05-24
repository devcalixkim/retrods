#ifndef RETRODS_SOUND_H
#define RETRODS_SOUND_H

void soundInit(void);
void soundPlayNav(void);    // short 440 Hz tick on D-pad move
void soundPlaySelect(void); // 880 Hz confirm on A / tap-release
void soundStep(void);       // call once per frame to stop expired channels

#endif
