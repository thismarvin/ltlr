#pragma once

#include <stdint.h>

typedef uint64_t u64;
typedef double f64;
typedef int32_t i32;

typedef struct
{
	u64 seed;
} Rng;

// Creates a simple wrapper around wyhash's API.
Rng RngCreate(u64 seed);

// Returns a pseudo-random u64.
u64 RngNextU64(Rng* self);

// Returns a pseudo-random f64 within the range [0, 1).
f64 RngNextF64(Rng* self);

// Returns a pseudo-random i32 within the range [minimum, maximum).
i32 RngNextRange(Rng* self, i32 minimum, i32 maximum);
