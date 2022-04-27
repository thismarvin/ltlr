#include "events.h"
#include "scene.h"
#include <raymath.h>

EventHandler EventHandlerCreate()
{
    return (EventHandler)
    {
        .listeners = DEQUE_OF(OnRaise),
    };
}

void EventHandlerSubscribe(EventHandler* self, const OnRaise onRaise)
{
    DequePushFront(&self->listeners, &onRaise);
}

void EventHandlerRaise(const EventHandler* self, const void* arguments)
{
    for (usize i = 0; i < DequeGetSize(&self->listeners); ++i)
    {
        const OnRaise* onRaise = &DEQUE_GET_UNCHECKED(&self->listeners, OnRaise, i);

        (*onRaise)(arguments);
    }
}

void EventHandlerDestroy(EventHandler* self)
{
    DequeDestroy(&self->listeners);
}

// TODO(thismarvin): Is this too specialized?
void SpawnCloudParticles(const EventCloudParticleParams* params)
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
