#pragma once

#include <stdint.h>

typedef struct
{
	uint64_t seed;
} Rng;

// Creates a simple wrapper around wyhash's API.
Rng RngCreate(uint64_t seed);

// Returns a pseudo-random u64.
uint64_t RngNextU64(Rng* self);

// Returns a pseudo-random f64 within the range [0, 1).
double RngNextF64(Rng* self);

// Returns a pseudo-random i32 within the range [minimum, maximum).
int RngNextRange(Rng* self, int minimum, int maximum);
