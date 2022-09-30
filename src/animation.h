#pragma once

#include "sprites_generated.h"

typedef enum
{
    ANIMATION_PLAYER_RUN = 0,
    ANIMATION_PLAYER_JUMP = 1,
    ANIMATION_PLAYER_SPIN = 2,
    ANIMATION_WALKER_IDLE = 3,
} Animation;

#define ANIMATION_PLAYER_RUN_LENGTH 4
#define ANIMATION_PLAYER_RUN_FRAME_DURATION 0.18f
extern const Sprite ANIMATION_PLAYER_RUN_FRAMES[ANIMATION_PLAYER_RUN_LENGTH];

#define ANIMATION_PLAYER_JUMP_LENGTH 5
#define ANIMATION_PLAYER_JUMP_FRAME_DURATION 0.075f
extern const Sprite ANIMATION_PLAYER_JUMP_FRAMES[ANIMATION_PLAYER_JUMP_LENGTH];

#define ANIMATION_PLAYER_SPIN_LENGTH 13
#define ANIMATION_PLAYER_SPIN_FRAME_DURATION 0.1f
extern const Sprite ANIMATION_PLAYER_SPIN_FRAMES[ANIMATION_PLAYER_SPIN_LENGTH];

#define ANIMATION_WALKER_IDLE_LENGTH 4
#define ANIMATION_WALKER_IDLE_FRAME_DURATION 0.2f
extern const Sprite ANIMATION_WALKER_IDLE_FRAMES[ANIMATION_WALKER_IDLE_LENGTH];

#define ANIMATIONS_LENGTH 4
extern const Sprite* ANIMATIONS[ANIMATIONS_LENGTH];
