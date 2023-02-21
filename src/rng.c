#include "rng.h"

#include <wyhash.h>

Rng RngCreate(const u64 seed)
{
	return (Rng) {
		.seed = seed,
	};
}

u64 RngNextU64(Rng* self)
{
	return wyrand(&self->seed);
}

f64 RngNextF64(Rng* self)
{
	const u64 value = RngNextU64(self);

	return wy2u01(value);
}

i32 RngNextRange(Rng* self, const i32 minimum, const i32 maximum)
{
	// TODO(thismarvin): This can definitely be better...

	const f64 step = RngNextF64(self);

	return minimum + (maximum - minimum) * step;
}
