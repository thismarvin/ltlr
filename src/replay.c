#include "replay.h"

#include "bit_mask.h"
#include "bytes.h"

#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIGNATURE_LENGTH (5)
#define SIGNATURE_SIZE (5)

static const char signature[SIGNATURE_LENGTH] = "ltlrr";

const char* StringFromReplayError(const ReplayError error)
{
	switch (error)
	{
		case REPLAY_ERROR_INVALIDATED_INPUT_STREAM: {
			return "The given InputStream cannot be a valid Replay as it has wrapped around on "
				   "itself.";
		};
		case REPLAY_ERROR_TOO_FEW_BYTES: {
			return "Too few bytes were given to possibly construct a Replay.";
		};
		case REPLAY_ERROR_SIGNATURE_MISMATCH: {
			return "The first few bytes do no match the signature of a valid Replay file.";
		};
		default: {
			return "Unknown error type.";
		}
	}
}

InputStream InputStreamCreate(const u8 totalBindings, const u32 capacity)
{
	assert(capacity <= MAX_REPLAY_LENGTH);

	return (InputStream) {
		.bits = BitMaskCreate(totalBindings, capacity),
		.totalBindings = totalBindings,
		.capacity = capacity,
		.length = 0,
		.barriers = calloc(totalBindings, sizeof(usize)),
	};
}

bool InputStreamLoadReplay(InputStream* self, const Replay* replay)
{
	if (replay->totalBindings > self->totalBindings || replay->length >= self->capacity)
	{
		return false;
	}

	memcpy(self->bits.contents, replay->bits.contents, replay->bits.size);

	self->length = replay->length;
	memset(self->barriers, 0, self->totalBindings);

	return true;
}

static usize WrapFrame(const InputStream* self, const u32 frame, const i32 offset)
{
	// Note that casting offset to an u32 only works because we also add capacity.
	return (frame + (u32)offset + self->capacity) % self->capacity;
}

void InputStreamPush(InputStream* self, const bool* payload)
{
	const usize wrapped = WrapFrame(self, self->length, 0);

	for (usize i = 0; i < self->totalBindings; ++i)
	{
		const usize binding = i;
		const bool isBindingPressed = payload[binding];

		BitMaskSet(&self->bits, binding, wrapped, isBindingPressed);
	}

	self->length += 1;
}

bool InputStreamPressing(const InputStream* self, const u8 binding, const u32 frame)
{
	const usize wrapped = WrapFrame(self, frame, 0);

	return BitMaskGet(&self->bits, binding, wrapped);
}

bool InputStreamPressed(
	const InputStream* self,
	const u8 binding,
	const usize buffer,
	const u32 frame
)
{
	const usize wrapped = WrapFrame(self, frame, 0);

	// Make sure the player is currently holding down the button.
	if (!InputStreamPressing(self, binding, wrapped))
	{
		return false;
	}

	// If the player was not holding down the button at any time in the range
	// [frame - buffer, frame) then the button can be considered just pressed.
	for (usize i = 1; i <= buffer; ++i)
	{
		const usize offsetFrame = WrapFrame(self, wrapped, -i);

		// Make sure the pressed event has not already been consumed.
		if (frame - i < self->barriers[binding])
		{
			return false;
		}

		if (!InputStreamPressing(self, binding, offsetFrame))
		{
			return true;
		}
	}

	// The player has held the button down for too long to be considered just pressed.
	return false;
}

bool InputStreamReleased(
	const InputStream* self,
	const u8 binding,
	const usize buffer,
	const u32 frame
)
{
	const usize wrapped = WrapFrame(self, frame, 0);

	// Make sure the player is not currently holding down the button.
	if (InputStreamPressing(self, binding, wrapped))
	{
		return false;
	}

	// If the player was holding down the button at any time in the range [frame - buffer, frame)
	// then the button can be considered just released.
	for (usize i = 1; i <= buffer; ++i)
	{
		const usize offsetFrame = WrapFrame(self, wrapped, -i);

		// Make sure the released event has not already been consumed.
		if (frame - i < self->barriers[binding])
		{
			return false;
		}

		if (InputStreamPressing(self, binding, offsetFrame))
		{
			return true;
		}
	}

	// The player has not pressed the button recently enough to be considered just released.
	return false;
}

void InputStreamConsume(InputStream* self, const u8 binding, const u32 frame)
{
	self->barriers[binding] = frame;
}

void InputStreamDestroy(InputStream* self)
{
	BitMaskDestroy(&self->bits);

	free(self->barriers);
}

ReplayResult ReplayTryFromInputStream(const u32 seed, const InputStream* stream)
{
	if (stream->length >= stream->capacity)
	{
		return (ReplayResult) {
			.type = REPLAY_RESULT_TYPE_ERR,
			.contents.err = REPLAY_ERROR_INVALIDATED_INPUT_STREAM,
		};
	}

	const u8 totalBindings = stream->totalBindings;
	const u32 length = stream->length;
	const BitMask bits = BitMaskCreate(totalBindings, stream->length);

	memcpy(bits.contents, stream->bits.contents, bits.size);

	return (ReplayResult) {
		.type = REPLAY_RESULT_TYPE_OK,
		.contents.ok =
			(Replay) {
				.seed = seed,
				.totalBindings = totalBindings,
				.length = length,
				.bits = bits,
			},
	};
}

ReplayResult ReplayTryFromBytes(const u8* data, const usize size)
{
	if (size < SIGNATURE_SIZE)
	{
		return (ReplayResult) {
			.type = REPLAY_RESULT_TYPE_ERR,
			.contents.err = REPLAY_ERROR_TOO_FEW_BYTES,
		};
	}

	u8* head = (u8*)data;

	if (memcmp(head, signature, SIGNATURE_SIZE) != 0)
	{
		return (ReplayResult) {
			.type = REPLAY_RESULT_TYPE_ERR,
			.contents.err = REPLAY_ERROR_SIGNATURE_MISMATCH,
		};
	}
	head += SIGNATURE_SIZE;

	u32 seed;
	{
		const usize tmp = sizeof(seed);
		memcpy(&seed, head, tmp);
		head += tmp;

		// Note that the replay stores seed using big-endian byte ordering.
		seed = U32FromBigEndian(seed);
	}

	u8 totalBindings;
	{
		const usize tmp = sizeof(totalBindings);
		memcpy(&totalBindings, head, tmp);
		head += tmp;
	}

	u32 length;
	{
		const usize tmp = sizeof(length);
		memcpy(&length, head, tmp);
		head += tmp;

		// Note that the replay stores length using big-endian byte ordering.
		length = U32FromBigEndian(length);
	}

	const BitMask bits = BitMaskCreate(totalBindings, length);
	{
		memcpy(bits.contents, head, bits.size);
		head += bits.size;
	}

	return (ReplayResult) {
		.type = REPLAY_RESULT_TYPE_OK,
		.contents.ok =
			(Replay) {
				.seed = seed,
				.totalBindings = totalBindings,
				.length = length,
				.bits = bits,
			},
	};
}

void ReplayDestroy(Replay* self)
{
	BitMaskDestroy(&self->bits);
}

ReplayBytes ReplayBytesFromReplay(const Replay* replay)
{
	const u32 seed = replay->seed;
	const u8 totalBindings = replay->totalBindings;
	const u32 length = replay->length;

	// clang-format off
	const usize size = 0
		+ sizeof(signature)
		+ sizeof(seed)
		+ sizeof(totalBindings)
		+ sizeof(length)
		+ replay->bits.size;
	// clang-format on

	u8* data = malloc(size);
	u8* head = data;

	{
		const usize tmp = sizeof(signature);
		memcpy(head, signature, tmp);
		head += tmp;
	}
	{
		const u32 seedButInBigEndian = U32ToBigEndian(seed);

		const usize tmp = sizeof(seedButInBigEndian);
		memcpy(head, &seedButInBigEndian, tmp);
		head += tmp;
	}
	{
		const usize tmp = sizeof(totalBindings);
		memcpy(head, &totalBindings, tmp);
		head += tmp;
	}
	{
		const u32 lengthButInBigEndian = U32ToBigEndian(length);

		const usize tmp = sizeof(lengthButInBigEndian);
		memcpy(head, &lengthButInBigEndian, tmp);
		head += tmp;
	}
	{
		const usize tmp = replay->bits.size;
		memcpy(head, replay->bits.contents, tmp);
		head += tmp;
	}

	return (ReplayBytes) {
		.data = data,
		.size = size,
	};
}

void ReplayBytesDestroy(ReplayBytes* self)
{
	free(self->data);
}
