#include "animation.h"

const char* ANIMATION_PLAYER_RUN_FRAMES[ANIMATION_PLAYER_RUN_LENGTH] =
{
    "player_run_0",
    "player_run_1",
    "player_run_2",
    "player_run_3",
};
const char* ANIMATION_PLAYER_JUMP_FRAMES[ANIMATION_PLAYER_JUMP_LENGTH] =
{
    "player_jump_0",
    "player_jump_1",
    "player_jump_2",
    "player_jump_3",
    "player_jump_4",
};
const char* ANIMATION_PLAYER_SPIN_FRAMES[ANIMATION_PLAYER_SPIN_LENGTH] =
{
    "player_spin_0",
    "player_spin_1",
    "player_spin_2",
    "player_spin_3",
    "player_spin_4",
    "player_spin_5",
    "player_spin_6",
    "player_spin_7",
    "player_spin_8",
    "player_spin_9",
    "player_spin_10",
    "player_spin_11",
    "player_spin_12",
};
const char* ANIMATION_WALKER_IDLE_FRAMES[ANIMATION_WALKER_IDLE_LENGTH] =
{
    "walker_idle_0",
    "walker_idle_1",
    "walker_idle_2",
    "walker_idle_3",
};

const char** ANIMATIONS[ANIMATIONS_LENGTH] =
{
    ANIMATION_PLAYER_RUN_FRAMES,
    ANIMATION_PLAYER_JUMP_FRAMES,
    ANIMATION_PLAYER_SPIN_FRAMES,
    ANIMATION_WALKER_IDLE_FRAMES,
};
