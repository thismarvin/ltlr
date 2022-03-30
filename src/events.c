#include "events.h"

void EventCollisionInit(Event* self, usize entity, usize otherEntity)
{
    self->tag = EVENT_COLLISION;
    self->entity = entity;
    self->collisionInner = (EventCollisionInner)
    {
        .otherEntity = otherEntity,
    };
}

void EventDamageInit(Event* self, usize entity, usize otherEntity)
{
    self->tag = EVENT_DAMAGE;
    self->entity = entity;
    self->damageInner = (EventDamageInner)
    {
        .otherEntity = otherEntity,
    };
}

void EventCloudParticleInit(Event* self, usize entity, int spawnCount)
{
    self->tag = EVENT_CLOUD_PARTICLE;
    self->entity = entity;

    self->cloudParticleInner = (EventCloudParticleInner)
    {
        .spawnCount = spawnCount,
    };
}
