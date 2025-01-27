#pragma once

// TODO(thismarvin): Add support for resizing? Probably use InputStreamParams...

#include "bit_mask.h"

#include <stdbool.h>
#include <stdint.h>

#define MUST_USE __attribute__((warn_unused_result))

// Derived by solving for x in the given equation: ((2^8 - 1) * x) / 64 = 2^32 - 1
#define MAX_REPLAY_LENGTH (1077952576)

typedef uint32_t u32;

typedef enum
{
	REPLAY_RESULT_TYPE_ERR,
	REPLAY_RESULT_TYPE_OK,
} ReplayResultType;

typedef enum
{
	REPLAY_ERROR_INVALIDATED_INPUT_STREAM,
	REPLAY_ERROR_TOO_FEW_BYTES,
	REPLAY_ERROR_SIGNATURE_MISMATCH,
} ReplayError;

typedef struct
{
	u8 totalBindings;
	u32 capacity;
	u32 length;
	u32* barriers;
	BitMask bits;
} InputStream;

typedef struct
{
	u32 seed;
	u8 totalBindings;
	u32 length;
	BitMask bits;
} Replay;

typedef struct
{
	ReplayResultType type;

	union {
		ReplayError err;
		Replay ok;
	} contents;
} ReplayResult;

typedef struct
{
	void* data;
	usize size;
} ReplayBytes;

const char* StringFromReplayError(ReplayError error);

InputStream InputStreamCreate(u8 totalBindings, u32 capacity);
bool InputStreamLoadReplay(InputStream* self, const Replay* replay) MUST_USE;
void InputStreamPush(InputStream* self, const bool* payload);
bool InputStreamPressing(const InputStream* self, u8 binding, u32 frame);
bool InputStreamPressed(const InputStream* self, u8 binding, usize buffer, u32 frame);
bool InputStreamReleased(const InputStream* self, u8 binding, usize buffer, u32 frame);
void InputStreamConsume(InputStream* self, u8 binding, u32 frame);
void InputStreamDestroy(InputStream* self);

ReplayResult ReplayTryFromInputStream(u32 seed, const InputStream* stream);
ReplayResult ReplayTryFromBytes(const u8* data, usize size);
void ReplayDestroy(Replay* self);

ReplayBytes ReplayBytesFromReplay(const Replay* replay);
void ReplayBytesDestroy(ReplayBytes* self);
