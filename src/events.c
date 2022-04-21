#include "events.h"
#include "scene.h"
#include <raymath.h>

// TODO(thismarvin): Is this too specialized?
void RaiseSpawnCloudParticleEvent(const EventCloudParticleParams* params)
{
    for (usize i = 0; i < params->spawnCount; ++i)
    {
        const Vector2 spreadOffset = (Vector2)
        {
            .x = (f32)GetRandomValue(-params->spread, params->spread),
            .y = 0,
        };
        const Vector2 center = Vector2Add(params->anchor, spreadOffset);
        const f32 rotation = (f32)GetRandomValue(DEG2RAD * -45, DEG2RAD * 45);
        const Vector2 direction = Vector2Rotate(params->direction, rotation);

        SceneDeferAddEntity(params->scene, ECreateCloudParticle(center.x, center.y, direction));
    }
}
