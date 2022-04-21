#include "events.h"

void EventCloudParticleInit(Event* self, const usize entity, const u16 spawnCount)
{
    self->tag = EVENT_CLOUD_PARTICLE;
    self->entity = entity;

    self->cloudParticleInner = (EventCloudParticleInner)
    {
        .spawnCount = spawnCount,
    };
}
