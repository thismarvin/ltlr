#pragma once

#include "common.h"
#include "easing.h"

#include <raylib.h>
#include <stdbool.h>

typedef enum
{
	FADE_IN,
	FADE_OUT,
} FadeType;

typedef struct
{
	Easer easer;
	Color color;
	f32 previous;
	f32 current;
	FadeType type;
} Fader;

Fader FaderDefault(void);
bool FaderIsDone(const Fader* self);
void FaderReset(Fader* self);
void FaderUpdate(Fader* self);
void FaderDraw(const Fader* self);
