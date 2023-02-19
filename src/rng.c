#include "rng.h"

#include <stdint.h>
#include <wyhash.h>

Rng RngCreate(const uint64_t seed)
{
	return (Rng) {
		.seed = seed,
	};
}

uint64_t RngNextU64(Rng* self)
{
	return wyrand(&self->seed);
}

double RngNextF64(Rng* self)
{
	const uint64_t value = RngNextU64(self);

	return wy2u01(value);
}

int RngNextRange(Rng* self, const int minimum, const int maximum)
{
	// TODO(thismarvin): This can definitely be better...

	const double step = RngNextF64(self);

	return minimum + (maximum - minimum) * step;
}
