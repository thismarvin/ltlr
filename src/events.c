#include "events.h"

void EventCollisionInit(Event* self, usize entity, usize otherEntity, Rectangle overlap)
{
    self->tag = EVENT_COLLISION;
    self->entity = entity;
    self->collisionInner = (EventCollisionInner)
    {
        .otherEntity = otherEntity,
        .overlap = overlap,
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