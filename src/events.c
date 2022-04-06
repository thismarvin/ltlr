#include "events.h"

void EventCollisionInit(Event* self, const usize entity, const usize otherEntity)
{
    self->tag = EVENT_COLLISION;
    self->entity = entity;
    self->collisionInner = (EventCollisionInner)
    {
        .otherEntity = otherEntity,
    };
}

void EventDamageInit(Event* self, const usize entity, const usize otherEntity)
{
    self->tag = EVENT_DAMAGE;
    self->entity = entity;
    self->damageInner = (EventDamageInner)
    {
        .otherEntity = otherEntity,
    };
}

void EventCloudParticleInit(Event* self, const usize entity, const u16 spawnCount)
{
    self->tag = EVENT_CLOUD_PARTICLE;
    self->entity = entity;

    self->cloudParticleInner = (EventCloudParticleInner)
    {
        .spawnCount = spawnCount,
    };
}
