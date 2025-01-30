#include "common.h"

#include "../../common.h"

#include <raylib.h>

Rectangle ApplyResolutionPerfectly(
	const Rectangle aabb,
	const Rectangle otherAabb,
	const Vector2 resolution
)
{
	Rectangle result = aabb;

	if (resolution.x < 0)
	{
		result.x = RectangleLeft(otherAabb) - aabb.width;
	}
	else if (resolution.x > 0)
	{
		result.x = RectangleRight(otherAabb);
	}

	if (resolution.y < 0)
	{
		result.y = RectangleTop(otherAabb) - aabb.height;
	}
	else if (resolution.y > 0)
	{
		result.y = RectangleBottom(otherAabb);
	}

	return result;
}
