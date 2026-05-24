#ifndef RETRODS_FONT_H
#define RETRODS_FONT_H

#include <nds.h>

// 8x8 bitmap font, MSB-first per row, 8 rows per glyph.
// Indexed by ASCII code 0x20..0x7E. Chars outside this range render blank.
extern const unsigned char font8x8[96][8];

#endif
