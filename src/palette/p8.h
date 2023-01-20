#pragma once

#include "../common.h"

#define P8_PALETTE_LENGTH (16)

#define P8_BLACK (Color) { 0, 0, 0, 255 }
#define P8_DARK_BLUE (Color) { 29, 43, 83, 255 }
#define P8_DARK_PURPLE (Color) { 126, 37, 83, 255 }
#define P8_DARK_GREEN (Color) { 0, 135, 81, 255 }
#define P8_BROWN (Color) { 171, 82, 54, 255 }
#define P8_DARK_GRAY (Color) { 95, 87, 79, 255 }
#define P8_LIGHT_GRAY (Color) { 194, 195, 199, 255 }
#define P8_WHITE (Color) { 255, 241, 232, 255 }
#define P8_RED (Color) { 255, 0, 77, 255 }
#define P8_ORANGE (Color) { 255, 163, 0, 255 }
#define P8_YELLOW (Color) { 255, 236, 39, 255 }
#define P8_GREEN (Color) { 0, 228, 54, 255 }
#define P8_BLUE (Color) { 41, 173, 255, 255 }
#define P8_LAVENDER (Color) { 131, 118, 156, 255 }
#define P8_PINK (Color) { 255, 119, 168, 255 }
#define P8_LIGHT_PEACH (Color) { 255, 204, 170, 255 }

Color P8PaletteGet(usize index);
