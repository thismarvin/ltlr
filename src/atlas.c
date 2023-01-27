#include "atlas.h"

void AtlasDraw(const Atlas* self, const AtlasDrawParams* params)
{
	const AtlasEntry* entry = &self->entries[params->sprite];

	Vector2 position = params->position;
	Rectangle source = entry->destination;

	if ((params->reflection & REFLECTION_REVERSE_X_AXIS) == 0)
	{
		position.x += entry->source.x;
		position.x -= params->intramural.x;
	}
	else
	{
		position.x += entry->untrimmed.width - RectangleRight(entry->source);
		position.x -= entry->untrimmed.width - RectangleRight(params->intramural);

		source.width = -source.width;
	}

	if ((params->reflection & REFLECTION_REVERSE_Y_AXIS) == 0)
	{
		position.y += entry->source.y;
		position.y -= params->intramural.y;
	}
	else
	{
		position.y += entry->untrimmed.height - RectangleBottom(entry->source);
		position.y -= entry->untrimmed.height - RectangleBottom(params->intramural);

		source.height = -source.height;
	}

	DrawTextureRec(self->texture, source, position, params->tint);
}

void AtlasDestroy(Atlas* self)
{
	UnloadTexture(self->texture);
	free(self->entries);
}
