#include "p8.h"

Color P8PaletteGet(const usize index)
{
	const usize wrapper = index % P8_PALETTE_LENGTH;

	switch (wrapper)
	{
		case 0:
			return P8_BLACK;

		case 1:
			return P8_DARK_BLUE;

		case 2:
			return P8_DARK_PURPLE;

		case 3:
			return P8_DARK_GREEN;

		case 4:
			return P8_BROWN;

		case 5:
			return P8_DARK_GRAY;

		case 6:
			return P8_LIGHT_GRAY;

		case 7:
			return P8_WHITE;

		case 8:
			return P8_RED;

		case 9:
			return P8_ORANGE;

		case 10:
			return P8_YELLOW;

		case 11:
			return P8_GREEN;

		case 12:
			return P8_BLUE;

		case 13:
			return P8_LAVENDER;

		case 14:
			return P8_PINK;

		case 15:
			return P8_LIGHT_PEACH;
	}

	return P8_BLACK;
}
