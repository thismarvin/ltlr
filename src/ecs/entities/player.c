#include "../../palette/p8.h"
#include "cloud_particle.h"
#include "player.h"
#include <assert.h>
#include <raymath.h>
#include <stdio.h>

static const f32 moveSpeed = 200;
static const f32 jumpHeight = 16 * 3 + 6;
static const f32 jumpDuration = 0.4;
static const f32 jumpGravity = (2 * jumpHeight) / (jumpDuration* jumpDuration);
static const f32 defaultGravity = jumpGravity * 1.5;
static const f32 jumpVelocity = jumpGravity * jumpDuration;
static const f32 terminalVelocity = 500;
static const f32 timeToSprint = CTX_DT * 6;
static const f32 timeToStop = CTX_DT * 6;

static void PlayerStandstill(CPlayer* player, CKinetic* kinetic)
{
    player->sprintTimer = 0;
    player->sprintState = SPRINT_STATE_NONE;
    player->sprintForce.x = 0;
    player->sprintDirection = DIR_NONE;
    kinetic->velocity.x = 0;
}

static bool PlayerIsVulnerable(const CPlayer* player)
{
    return player->invulnerableTimer >= player->invulnerableDuration;
}

static void SpawnCloudParticle
(
    Scene* scene,
    const Vector2 position,
    const f32 radius,
    const Vector2 velocity,
    const Vector2 acceleration,
    const f32 lifetime
)
{
    CloudParticleBuilder* builder = malloc(sizeof(CloudParticleBuilder));
    builder->entity = SceneAllocateEntity(scene);
    builder->position = position;
    builder->radius = radius;
    builder->initialVelocity = velocity;
    builder->acceleration = acceleration;
    builder->lifetime = lifetime;
    SceneDefer(scene, CloudParticleCreate, builder);
}

static void PlayerSpawnImpactParticles(Scene* scene, const usize entity, const f32 groundY)
{
    assert(SceneEntityHasDependencies(scene, entity, TAG_POSITION | TAG_DIMENSION | TAG_KINETIC));

    const CPosition* position = &scene->components.positions[entity];
    const CDimension* dimension = &scene->components.dimensions[entity];
    const CKinetic* kinetic = &scene->components.kinetics[entity];

    static const f32 gravity = 9.8f;
    const usize spawnCount = GetRandomValue(10, 20);
    const f32 spread = dimension->width * 0.25;
    const Vector2 leftAnchor = (Vector2)
    {
        .x = position->value.x,
        .y = groundY,
    };
    const Vector2 rightAnchor = (Vector2)
    {
        .x = position->value.x + dimension->width,
        .y = groundY,
    };

    // Lateral pockets.
    {
        static const f32 theta = 25;
        const f32 angleIncrement = (DEG2RAD * theta) / spawnCount;

        for (usize i = 0; i < spawnCount; ++i)
        {
            const f32 radius = GetRandomValue(1, 4);
            const f32 offset = GetRandomValue(0, spread);
            const f32 speed = GetRandomValue(10, 30);
            const f32 lifetime = 1 + 0.5 * GetRandomValue(0, 4);

            // Left pocket.
            {
                const Vector2 cloudPosition = (Vector2)
                {
                    .x = leftAnchor.x - offset - radius * 2,
                    .y = leftAnchor.y - radius * 2,
                };
                const f32 rotation = (DEG2RAD * 180) + angleIncrement * i;
                const Vector2 direction = (Vector2)
                {
                    .x = cosf(rotation),
                    .y = sinf(rotation),
                };
                const Vector2 vo = Vector2Scale(direction, speed);
                const Vector2 ao = (Vector2)
                {
                    // a = (vf - vo) / t
                    .x = (0 - vo.x) / lifetime,
                    .y = gravity,
                };

                SpawnCloudParticle(scene, cloudPosition, radius, vo, ao, lifetime);
            }

            // Right pocket.
            {
                const Vector2 cloudPosition = (Vector2)
                {
                    .x = rightAnchor.x + offset,
                    .y = rightAnchor.y - radius * 2,
                };
                const f32 rotation = 0 - angleIncrement * i;
                const Vector2 direction = (Vector2)
                {
                    .x = cosf(rotation),
                    .y = sinf(rotation),
                };
                const Vector2 vo = Vector2Scale(direction, speed);
                const Vector2 ao = (Vector2)
                {
                    // a = (vf - vo) / t
                    .x = (0 - vo.x) / lifetime,
                    .y = gravity,
                };

                SpawnCloudParticle(scene, cloudPosition, radius, vo, ao, lifetime);
            }
        }
    }

    if (kinetic->velocity.x == 0)
    {
        return;
    }

    // Spawn extra cloud particles in the direction opposite of velocity.
    {
        static const f32 reflection = 30;
        static const f32 theta = 20;
        const f32 anchorOffset = dimension->width * 0.25;
        const usize total = spawnCount * 0.5;
        const f32 angleIncrement = (DEG2RAD * theta) / total;

        for (usize i = 0; i < total; ++i)
        {
            const f32 radius = GetRandomValue(1, 3);
            const f32 offset = GetRandomValue(0, spread);
            const f32 speed = GetRandomValue(20, 35);
            const f32 lifetime = 0.5 + 0.5 * GetRandomValue(0, 4);

            if (kinetic->velocity.x > 0)
            {
                const Vector2 cloudPosition = (Vector2)
                {
                    .x = leftAnchor.x + anchorOffset - offset - radius * 2,
                    .y = leftAnchor.y - radius * 2,
                };
                const f32 rotation = (DEG2RAD * (180 + reflection - theta * 0.5)) + angleIncrement * i;
                const Vector2 direction = (Vector2)
                {
                    .x = cosf(rotation),
                    .y = sinf(rotation),
                };
                const Vector2 vo = Vector2Scale(direction, speed);
                const Vector2 ao = (Vector2)
                {
                    // a = (vf - vo) / t
                    .x = (0 - vo.x) / lifetime,
                    .y = gravity,
                };

                SpawnCloudParticle(scene, cloudPosition, radius, vo, ao, lifetime);
            }
            else
            {
                const Vector2 cloudPosition = (Vector2)
                {
                    .x = rightAnchor.x - anchorOffset + offset,
                    .y = rightAnchor.y - radius * 2,
                };
                const f32 rotation = (DEG2RAD * (0 - reflection + theta * 0.5)) - angleIncrement * i;
                const Vector2 direction = (Vector2)
                {
                    .x = cosf(rotation),
                    .y = sinf(rotation),
                };
                const Vector2 vo = Vector2Scale(direction, speed);
                const Vector2 ao = (Vector2)
                {
                    // a = (vf - vo) / t
                    .x = (0 - vo.x) / lifetime,
                    .y = gravity,
                };

                SpawnCloudParticle(scene, cloudPosition, radius, vo, ao, lifetime);
            }
        }
    }
}

static void PlayerSpawnJumpParticles(Scene* scene, const usize entity)
{
    assert(SceneEntityHasDependencies(scene, entity, TAG_POSITION | TAG_DIMENSION | TAG_KINETIC));

    const CPosition* position = &scene->components.positions[entity];
    const CDimension* dimension = &scene->components.dimensions[entity];
    const CKinetic* kinetic = &scene->components.kinetics[entity];

    static const f32 gravity = 9.8f;
    const usize spawnCount = GetRandomValue(10, 30);
    const Vector2 anchor = (Vector2)
    {
        .x = position->value.x + dimension->width * 0.5,
        .y = position->value.y + dimension->height,
    };

    // Lateral pockets.
    {
        static const f32 theta = 30;
        const f32 angleIncrement = (DEG2RAD * theta) / spawnCount;

        for (usize i = 0; i < spawnCount; ++i)
        {
            const f32 radius = GetRandomValue(1, 3);
            const f32 speed = GetRandomValue(10, 15);
            const f32 lifetime = 0.5 + 0.5 * GetRandomValue(0, 3);

            // Left pocket.
            {
                const Vector2 cloudPosition = (Vector2)
                {
                    .x = anchor.x - radius * 2,
                    .y = anchor.y - radius * 2,
                };
                const f32 rotation = (DEG2RAD * 180) + angleIncrement * i;
                const Vector2 direction = (Vector2)
                {
                    .x = cosf(rotation),
                    .y = sinf(rotation),
                };
                const Vector2 vo = Vector2Scale(direction, speed);
                const Vector2 ao = (Vector2)
                {
                    // a = (vf - vo) / t
                    .x = (0 - vo.x) / lifetime,
                    .y = gravity,
                };

                SpawnCloudParticle(scene, cloudPosition, radius, vo, ao, lifetime);
            }

            // Right pocket.
            {
                const Vector2 cloudPosition = (Vector2)
                {
                    .x = anchor.x,
                    .y = anchor.y - radius * 2,
                };
                const f32 rotation = 0 - angleIncrement * i;
                const Vector2 direction = (Vector2)
                {
                    .x = cosf(rotation),
                    .y = sinf(rotation),
                };
                const Vector2 vo = Vector2Scale(direction, speed);
                const Vector2 ao = (Vector2)
                {
                    // a = (vf - vo) / t
                    .x = (0 - vo.x) / lifetime,
                    .y = gravity,
                };

                SpawnCloudParticle(scene, cloudPosition, radius, vo, ao, lifetime);
            }
        }
    }

    // Middle pocket.
    {
        static const f32 reflection = 20;
        static const f32 theta = 20;
        static const f32 lateralMultiplier = 1.75;

        const i8 sign = SIGN(kinetic->velocity.x);
        f32 angle = -90;

        if (sign < 0)
        {
            angle = -reflection;
        }
        else if (sign > 0)
        {
            angle = -180 + reflection;
        }

        angle -= theta * 0.5;
        angle *= DEG2RAD;

        const usize total = spawnCount * 0.5;
        const f32 angleIncrement = (DEG2RAD * theta) / total;

        for (usize i = 0; i < total; ++i)
        {
            const f32 radius = GetRandomValue(2, 3);
            const f32 lifetime = 0.5 + 0.5 * GetRandomValue(0, 4);

            const Vector2 cloudPosition = (Vector2)
            {
                .x = anchor.x - radius,
                .y = anchor.y - radius * 2,
            };
            const f32 rotation = angle + angleIncrement * i;
            const Vector2 directionTmp = (Vector2)
            {
                .x = cosf(rotation),
                .y = sinf(rotation),
            };

            f32 speed = GetRandomValue(10, 15);

            if (kinetic->velocity.x != 0)
            {
                speed *= lateralMultiplier;
            }

            const Vector2 vo = Vector2Scale(directionTmp, speed);
            const Vector2 ao = (Vector2)
            {
                // a = (vf - vo) / t
                .x = (0 - vo.x) / lifetime,
                .y = gravity,
            };

            SpawnCloudParticle(scene, cloudPosition, radius, vo, ao, lifetime);
        }
    }
}

static void PlayerOnDamage(Scene* scene, const usize entity, const usize otherEntity)
{
    assert(SceneEntityHasDependencies(scene, entity, TAG_PLAYER | TAG_MORTAL));
    assert(SceneEntityHasDependencies(scene, otherEntity, TAG_DAMAGE));

    CPlayer* player = &scene->components.players[entity];
    CMortal* mortal = &scene->components.mortals[entity];

    const CDamage* otherDamage = &scene->components.damages[otherEntity];

    if (!PlayerIsVulnerable(player))
    {
        return;
    }

    mortal->hp -= otherDamage->value;
    player->invulnerableTimer = 0;
}

static void PlayerOnCollision(const OnCollisionParams* params)
{
    static const u64 dependencies = TAG_PLAYER | TAG_MORTAL;
    assert(SceneEntityHasDependencies(params->scene, params->entity, dependencies));

    CMortal* mortal = &params->scene->components.mortals[params->entity];

    // Collision specific logic that will not resolve the player.
    {
        if (SceneEntityHasDependencies(params->scene, params->otherEntity, TAG_DAMAGE))
        {
            PlayerOnDamage(params->scene, params->entity, params->otherEntity);
        }

        if (SceneEntityHasDependencies(params->scene, params->otherEntity, TAG_BATTERY))
        {
            // TODO(thismarvin): Add a static PlayerIncrementHealth method?
            mortal->hp += 1;
            mortal->hp = MIN(mortal->hp, PLAYER_MAX_HIT_POINTS);

            SceneIncrementScore(params->scene, 100);

            SceneDeferDeallocateEntity(params->scene, params->otherEntity);
        }

        if (SceneEntityHasDependencies(params->scene, params->otherEntity, TAG_SOLAR_PANEL | TAG_SPRITE))
        {
            // TODO(thismarvin): Check if we have a battery.

            CSprite* otherSprite = &params->scene->components.sprites[params->otherEntity];

            otherSprite->type = SPRITE_SOLAR_0001;

            SceneDeferDisableTag(params->scene, params->otherEntity, TAG_SOLAR_PANEL);
        }
    }
}

static OnResolutionResult PlayerOnResolution(const OnResolutionParams* params)
{
    static const u64 dependencies = TAG_PLAYER | TAG_KINETIC;
    assert(SceneEntityHasDependencies(params->scene, params->entity, dependencies));

    CPlayer* player = &params->scene->components.players[params->entity];
    CKinetic* kinetic = &params->scene->components.kinetics[params->entity];

    // Collision specific logic that will not resolve the player.
    {
        if (SceneEntityHasDependencies(params->scene, params->otherEntity, TAG_WALKER))
        {
            return (OnResolutionResult)
            {
                .aabb = params->aabb,
            };
        }
    }

    // Collision leeway.
    {
        // Check if the player hit its head on the bottom of a collider.
        if (params->resolution.y > 0 && fabsf(params->overlap.width) <= 4)
        {
            Rectangle resolvedAabb = params->aabb;

            if (params->aabb.x < params->otherAabb.x)
            {
                resolvedAabb.x = RectangleLeft(params->otherAabb) - params->aabb.width;
            }
            else
            {
                resolvedAabb.x = RectangleRight(params->otherAabb);
            }

            return (OnResolutionResult)
            {
                .aabb = resolvedAabb,
            };
        }
    }

    // Make sure that landing on top of a one-way platform does not noticeably teleport the player.
    {
        if (kinetic->velocity.y > 0 && params->resolution.y < 0)
        {
            const f32 difference = fabsf(RectangleTop(params->otherAabb) - RectangleBottom(params->aabb));

            if (difference > 3.0f)
            {
                return (OnResolutionResult)
                {
                    .aabb = params->aabb,
                };
            }
        }
    }

    // Resolve collision.
    const Rectangle resolvedAabb = ApplyResolutionPerfectly(params->aabb, params->otherAabb,
                                   params->resolution);

    // Resolution specific player logic.
    {
        if ((params->resolution.x < 0 && kinetic->velocity.x > 0)
                || (params->resolution.x > 0 && kinetic->velocity.x < 0))
        {
            PlayerStandstill(player, kinetic);
        }

        if (params->resolution.y < 0)
        {
            player->grounded = true;

            if (kinetic->velocity.y > terminalVelocity * 0.75)
            {
                PlayerSpawnImpactParticles(params->scene, params->entity, RectangleTop(params->otherAabb));
            }
        }

        if ((params->resolution.y < 0 && kinetic->velocity.y > 0)
                || (params->resolution.y > 0 && kinetic->velocity.y < 0))
        {
            kinetic->velocity.y = 0;
        }
    }

    return (OnResolutionResult)
    {
        .aabb = resolvedAabb,
    };
}

void PlayerCreate(Scene* scene, const void* params)
{
    const PlayerBuilder* builder = params;

    const Vector2 position = Vector2Create(builder->x, builder->y);
    const Rectangle intramural = (Rectangle)
    {
        .x = 24,
        .y = 29,
        .width = 15,
        .height = 35,
    };

    scene->components.tags[builder->entity] =
        TAG_NONE
        | TAG_POSITION
        | TAG_DIMENSION
        | TAG_ANIMATION
        | TAG_KINETIC
        | TAG_SMOOTH
        | TAG_COLLIDER
        | TAG_PLAYER
        | TAG_MORTAL;

    scene->components.positions[builder->entity] = (CPosition)
    {
        .value = position,
    };

    scene->components.dimensions[builder->entity] = (CDimension)
    {
        .width = intramural.width,
        .height = intramural.height,
    };

    scene->components.animations[builder->entity] = (CAnimation)
    {
        .frameTimer = 0,
        .frameDuration = ANIMATION_PLAYER_STILL_FRAME_DURATION,
        .intramural = intramural,
        .reflection = REFLECTION_NONE,
        .type = ANIMATION_PLAYER_STILL,
        .frame = 0,
        .length = ANIMATION_PLAYER_STILL_LENGTH,
    };

    scene->components.kinetics[builder->entity] = (CKinetic)
    {
        .velocity = VECTOR2_ZERO,
        .acceleration = VECTOR2_ZERO,
    };

    scene->components.smooths[builder->entity] = (CSmooth)
    {
        .previous = position,
    };

    scene->components.colliders[builder->entity] = (CCollider)
    {
        .resolutionSchema = RESOLVE_NONE,
        .layer = LAYER_NONE,
        .mask = LAYER_TERRAIN | LAYER_LETHAL | LAYER_INTERACTABLE,
        .onCollision = PlayerOnCollision,
        .onResolution = PlayerOnResolution,
    };

    scene->components.mortals[builder->entity] = (CMortal)
    {
        .hp = 2,
    };

    static const f32 coyoteDuration = CTX_DT * 6;
    static const f32 invulnerableDuration = 1.5f;

    scene->components.players[builder->entity] = (CPlayer)
    {
        .groundedLastFrame = false,
        .grounded = false,
        .coyoteTimer = coyoteDuration,
        .coyoteDuration = coyoteDuration,
        .jumping = false,
        .dead = false,
        .gravityForce = VECTOR2_ZERO,
        .invulnerableTimer = invulnerableDuration,
        .invulnerableDuration = invulnerableDuration,
        .initialDirection = DIR_NONE,
        .sprintDirection = DIR_NONE,
        .sprintState = SPRINT_STATE_NONE,
        .sprintTimer = 0,
        .sprintDuration = 0,
        .sprintForce = VECTOR2_ZERO,
        .animationState = PLAYER_ANIMATION_STATE_STILL,
    };
}

static void PlayerDecelerate(CPlayer* player, const CKinetic* kinetic)
{
    if (kinetic->velocity.x == 0)
    {
        player->sprintTimer = 0;
        player->sprintState = SPRINT_STATE_NONE;
        player->sprintForce.x = 0;
        player->sprintDirection = DIR_NONE;

        return;
    }

    const f32 delta = fabsf(kinetic->velocity.x);

    static const f32 vf = 0;
    const f32 vo = kinetic->velocity.x;
    const f32 t = delta * timeToStop / moveSpeed;

    player->sprintTimer = 0;
    player->sprintDuration = t;
    player->sprintState = SPRINT_STATE_DECELERATING;
    // vf = vo + a * t
    // a = (vf - vo) / t
    player->sprintForce.x = (vf - vo) / t;
    player->sprintDirection = DIR_NONE;
}

static void PlayerAccelerate(CPlayer* player, const CKinetic* kinetic, const Direction direction)
{
    if ((direction == DIR_LEFT && kinetic->velocity.x <= -moveSpeed)
            || (direction == DIR_RIGHT && kinetic->velocity.x >= moveSpeed))
    {
        player->sprintTimer = 0;
        player->sprintState = SPRINT_STATE_TERMINAL;
        player->sprintForce.x = 0;
        player->sprintDirection = direction;

        return;
    }

    f32 delta = moveSpeed;

    if ((direction == DIR_LEFT && kinetic->velocity.x < 0)
            || (direction == DIR_RIGHT && kinetic->velocity.x > 0))
    {
        delta -= fabsf(kinetic->velocity.x);
    }
    else if ((direction == DIR_LEFT && kinetic->velocity.x > 0)
             || (direction == DIR_RIGHT && kinetic->velocity.x < 0))
    {
        delta += fabsf(kinetic->velocity.x);
    }

    const i8 sign = direction == DIR_LEFT ? -1 : 1;

    const f32 vf = moveSpeed * sign;
    const f32 vo = kinetic->velocity.x;
    const f32 t = delta * timeToSprint / moveSpeed;

    player->sprintTimer = 0;
    player->sprintDuration = t;
    player->sprintState = SPRINT_STATE_ACCELERATING;
    // vf = vo + a * t
    // a = (vf - vo) / t
    player->sprintForce.x = (vf - vo) / t;
    player->sprintDirection = direction;
}

static void PlayerLateralMovementLogic(const Scene* scene, CPlayer* player, CKinetic* kinetic)
{
    Direction strafe = DIR_NONE;

    // Input.
    if (!InputHandlerPressing(&scene->input, "right")
            && InputHandlerPressing(&scene->input, "left"))
    {
        player->initialDirection = DIR_LEFT;
        strafe = DIR_LEFT;
    }
    else if (!InputHandlerPressing(&scene->input, "left")
             && InputHandlerPressing(&scene->input, "right"))
    {
        player->initialDirection = DIR_RIGHT;
        strafe = DIR_RIGHT;
    }
    else if (player->initialDirection == DIR_RIGHT && InputHandlerPressing(&scene->input, "left"))
    {
        strafe = DIR_LEFT;
    }
    else if (player->initialDirection == DIR_LEFT && InputHandlerPressing(&scene->input, "right"))
    {
        strafe = DIR_RIGHT;
    }

    // Handle sprint state.
    switch (player->sprintState)
    {
        case SPRINT_STATE_NONE:
        {
            if (strafe == DIR_NONE)
            {
                player->sprintForce.x = 0;

                break;
            }

            PlayerAccelerate(player, kinetic, strafe);

            break;
        }

        case SPRINT_STATE_ACCELERATING:
        {
            if (strafe == DIR_NONE)
            {
                PlayerDecelerate(player, kinetic);

                break;
            }

            if ((player->sprintDirection == DIR_LEFT && strafe == DIR_RIGHT)
                    || (player->sprintDirection == DIR_RIGHT && strafe == DIR_LEFT))
            {
                PlayerAccelerate(player, kinetic, strafe);

                break;
            }

            player->sprintTimer += CTX_DT;

            if (player->sprintTimer >= player->sprintDuration)
            {
                player->sprintTimer = 0;
                player->sprintState = SPRINT_STATE_TERMINAL;
                player->sprintForce.x = 0;

                // const i8 sign = strafe == DIR_RIGHT ? 1 : -1;
                // kinetic->velocity.x = player->moveSpeed * sign;
            }

            break;
        }

        case SPRINT_STATE_TERMINAL:
        {
            if ((player->sprintDirection == DIR_LEFT && strafe == DIR_RIGHT)
                    || (player->sprintDirection == DIR_RIGHT && strafe == DIR_LEFT))
            {
                PlayerAccelerate(player, kinetic, strafe);

                break;
            }

            if (strafe == DIR_NONE)
            {
                PlayerDecelerate(player, kinetic);
            }

            break;
        }

        case SPRINT_STATE_DECELERATING:
        {
            if (strafe != DIR_NONE)
            {
                PlayerAccelerate(player, kinetic, strafe);

                break;
            }

            player->sprintTimer += CTX_DT;

            if (player->sprintTimer >= player->sprintDuration)
            {
                PlayerStandstill(player, kinetic);
            }

            break;
        }
    }
}

Direction Facing(const CPlayer* player)
{
    if (player->sprintDirection != DIR_NONE)
    {
        return player->sprintDirection;
    }

    return player->initialDirection;
}

void PlayerInputUpdate(Scene* scene, const usize entity)
{
    static const u64 dependencies = TAG_PLAYER | TAG_KINETIC;

    if (!SceneEntityHasDependencies(scene, entity, dependencies))
    {
        return;
    }

    if (scene->advanceStageRequested)
    {
        return;
    }

    CPlayer* player = &scene->components.players[entity];
    CKinetic* kinetic = &scene->components.kinetics[entity];

    if (player->dead)
    {
        return;
    }

    const bool coyoteTimeActive = player->coyoteTimer < player->coyoteDuration;

    // Maintenance.
    {
        player->groundedLastFrame = player->grounded;

        if (player->grounded)
        {
            kinetic->velocity.y = 0;
            player->jumping = false;
        }

        player->gravityForce.y = defaultGravity;

        if (player->jumping && kinetic->velocity.y < jumpVelocity)
        {
            player->gravityForce.y = jumpGravity;
        }

        if (coyoteTimeActive)
        {
            player->coyoteTimer += CTX_DT;
        }
    }

    PlayerLateralMovementLogic(scene, player, kinetic);

    // Jumping.
    {
        if ((player->grounded || coyoteTimeActive) && !player->jumping
                && InputHandlerPressed(&scene->input, "jump"))
        {
            InputHandlerConsume(&scene->input, "jump");

            player->grounded = false;
            player->jumping = true;
            kinetic->velocity.y = -jumpVelocity;

            PlayerSpawnJumpParticles(scene, entity);
        }

        // Variable Jump Height.
        if (InputHandlerReleased(&scene->input, "jump") && kinetic->velocity.y < 0)
        {
            InputHandlerConsume(&scene->input, "jump");

            player->jumping = false;
            kinetic->velocity.y = MAX(kinetic->velocity.y, -jumpVelocity * 0.5);
        }
    }

    // Assume that the player is not grounded; prove that it is later.
    player->grounded = false;

    // TODO(thismarvin): Should grounded logic really be in Input?

    // Calculate Net Force.
    {
        kinetic->acceleration = (Vector2)
        {
            .x = player->gravityForce.x + player->sprintForce.x,
            .y = player->gravityForce.y + player->sprintForce.y,
        };
    }
}

void PlayerPostCollisionUpdate(Scene* scene, const usize entity)
{
    static const u64 dependencies = TAG_PLAYER | TAG_POSITION | TAG_KINETIC;

    if (!SceneEntityHasDependencies(scene, entity, dependencies))
    {
        return;
    }

    CPlayer* player = &scene->components.players[entity];
    CPosition* position = &scene->components.positions[entity];
    CKinetic* kinetic = &scene->components.kinetics[entity];

    // General purpose player specific collision logic.
    {
        // Keep the player's x within the scene's bounds.
        if (position->value.x < scene->bounds.x)
        {
            position->value.x = scene->bounds.x;
            PlayerStandstill(player, kinetic);
        }
        else if (position->value.x > RectangleRight(scene->bounds))
        {
            kinetic->velocity.x = moveSpeed;

            SceneDeferAdvanceStage(scene);
        }
    }

    // Enable "Coyote Time" if the player walked off an edge.
    if (player->groundedLastFrame && !player->grounded)
    {
        player->coyoteTimer = 0;
    }
}

static void PlayerFlashingLogic(Scene* scene, const usize entity)
{
    CPlayer* player = &scene->components.players[entity];

    player->invulnerableTimer += CTX_DT;

    static const u32 totalFlashes = 5;
    const f32 timeSlice = player->invulnerableDuration / (totalFlashes * 2.0f);

    if (!player->dead && !PlayerIsVulnerable(player))
    {
        const u32 passedSlices = (u32)(player->invulnerableTimer / timeSlice);

        if (passedSlices % 2 == 0)
        {
            SceneDeferDisableTag(scene, entity, TAG_ANIMATION);
        }
        else
        {
            SceneDeferEnableTag(scene, entity, TAG_ANIMATION);
        }
    }
    else
    {
        // This is a pre-caution to make sure the last state isn't off.
        SceneDeferEnableTag(scene, entity, TAG_ANIMATION);
    }
}

void PlayerMortalUpdate(Scene* scene, const usize entity)
{
    static const u64 dependencies = TAG_PLAYER | TAG_MORTAL | TAG_POSITION | TAG_KINETIC;

    if (!SceneEntityHasDependencies(scene, entity, dependencies))
    {
        return;
    }

    CPlayer* player = &scene->components.players[entity];
    const CMortal* mortal = &scene->components.mortals[entity];
    const CPosition* position = &scene->components.positions[entity];
    CKinetic* kinetic = &scene->components.kinetics[entity];

    // TODO(thismarvin): This might need to be more involved in the future...
    if (position->value.y > CTX_VIEWPORT_HEIGHT * 2)
    {
        SceneDeferReset(scene);

        return;
    }

    if (player->dead)
    {
        return;
    }

    // Deal with the player falling out-of-bounds.
    if (position->value.y > scene->bounds.height)
    {
        player->dead = true;

        SceneDeferDisableTag(scene, entity, TAG_COLLIDER);

        kinetic->velocity.x = 0;
        kinetic->acceleration.x = 0;

        return;
    }

    // Deal with the player literally dying.
    if (mortal->hp <= 0)
    {
        player->dead = true;

        SceneDeferEnableTag(scene, entity, TAG_ANIMATION);
        SceneDeferDisableTag(scene, entity, TAG_COLLIDER);

        if (kinetic->velocity.y >= 0)
        {
            kinetic->velocity.y = -jumpVelocity * 0.85;
        }

        kinetic->acceleration.x = 0;

        return;
    }

    PlayerFlashingLogic(scene, entity);
}

static bool IsFrameJustStarting(const CAnimation* animation)
{
    return animation->frameTimer == 0;
}

static void EnableAnimation(Scene* scene, usize entity, CPlayer* player, Animation animation)
{
    CAnimation contents;

    switch (animation)
    {
        case ANIMATION_PLAYER_STILL:
        {
            contents = (CAnimation)
            {
                .frameTimer = 0,
                .frameDuration = ANIMATION_PLAYER_STILL_FRAME_DURATION,
                .intramural = (Rectangle) { 24, 29, 15, 35 },
                .reflection = REFLECTION_NONE,
                .frame = 0,
                .length = ANIMATION_PLAYER_STILL_LENGTH,
                .type = ANIMATION_PLAYER_STILL,
            };

            player->animationState = PLAYER_ANIMATION_STATE_STILL;

            break;
        }

        case ANIMATION_PLAYER_RUN:
        {
            contents = (CAnimation)
            {
                .frameTimer = 0,
                .frameDuration = ANIMATION_PLAYER_RUN_FRAME_DURATION,
                .intramural = (Rectangle) { 24, 29, 15, 35 },
                .reflection = REFLECTION_NONE,
                .frame = 0,
                .length = ANIMATION_PLAYER_RUN_LENGTH,
                .type = ANIMATION_PLAYER_RUN,
            };

            player->animationState = PLAYER_ANIMATION_STATE_RUNNING;

            break;
        }

        case ANIMATION_PLAYER_JUMP:
        {
            contents = (CAnimation)
            {
                .frameTimer = 0,
                .frameDuration = ANIMATION_PLAYER_JUMP_FRAME_DURATION,
                .intramural = (Rectangle) { 24, 29, 15, 35 },
                .reflection = REFLECTION_NONE,
                .frame = 0,
                .length = ANIMATION_PLAYER_JUMP_LENGTH,
                .type = ANIMATION_PLAYER_JUMP,
            };

            player->animationState = PLAYER_ANIMATION_STATE_JUMPING;

            break;
        }

        case ANIMATION_PLAYER_SPIN:
        {
            contents = (CAnimation)
            {
                .frameTimer = 0,
                .frameDuration = ANIMATION_PLAYER_SPIN_FRAME_DURATION,
                .intramural = (Rectangle) { 24, 29, 15, 35 },
                .reflection = REFLECTION_NONE,
                .frame = 0,
                .length = ANIMATION_PLAYER_SPIN_LENGTH,
                .type = ANIMATION_PLAYER_SPIN,
            };

            player->animationState = PLAYER_ANIMATION_STATE_SPINNING;

            break;
        }

        default:
        {
            fprintf(stderr, "Unsupported Animation type");
            exit(EXIT_FAILURE);
            break;
        }
    }

    scene->components.animations[entity] = contents;
}

void PlayerAnimationUpdate(Scene* scene, const usize entity)
{
    static const u64 dependencies = TAG_PLAYER;

    if (!SceneEntityHasDependencies(scene, entity, dependencies))
    {
        return;
    }

    CPlayer* player = &scene->components.players[entity];
    CAnimation* animation = &scene->components.animations[entity];

    // Deal with death.
    {
        if (player->dead && player->animationState != PLAYER_ANIMATION_STATE_DYING)
        {
            EnableAnimation(scene, entity, player, ANIMATION_PLAYER_SPIN);

            // This is a little hacky, but I don't want to add a bespoke death animation!
            animation->frameDuration = ANIMATION_PLAYER_SPIN_FRAME_DURATION * 0.5;
            player->animationState = PLAYER_ANIMATION_STATE_DYING;
        }
    }

    switch (player->animationState)
    {
        case PLAYER_ANIMATION_STATE_STILL:
        {
            if (player->jumping)
            {
                EnableAnimation(scene, entity, player, ANIMATION_PLAYER_JUMP);

                break;
            }

            if (player->sprintState != SPRINT_STATE_NONE)
            {
                EnableAnimation(scene, entity, player, ANIMATION_PLAYER_RUN);

                break;
            }

            // TODO(thismarvin): Implement an idle animation?

            break;
        }

        case PLAYER_ANIMATION_STATE_RUNNING:
        {
            if (player->jumping)
            {
                EnableAnimation(scene, entity, player, ANIMATION_PLAYER_JUMP);

                break;
            }

            if (player->sprintState == SPRINT_STATE_NONE)
            {
                if (IsFrameJustStarting(animation))
                {
                    EnableAnimation(scene, entity, player, ANIMATION_PLAYER_STILL);
                }

                break;
            }

            break;
        }

        case PLAYER_ANIMATION_STATE_JUMPING:
        {
            if (player->grounded)
            {
                if (player->sprintDirection != DIR_NONE)
                {
                    if (IsFrameJustStarting(animation))
                    {
                        EnableAnimation(scene, entity, player, ANIMATION_PLAYER_RUN);
                    }

                    break;
                }

                EnableAnimation(scene, entity, player, ANIMATION_PLAYER_STILL);

                break;
            }

            break;
        }

        case PLAYER_ANIMATION_STATE_SPINNING:
        {
            break;
        }

        case PLAYER_ANIMATION_STATE_DYING:
        {
            break;
        }
    }

    // Animation reflection logic.
    {
        const Direction facing = Facing(player);
        const Reflection reflection = facing == DIR_LEFT ? REFLECTION_REVERSE_X_AXIS : REFLECTION_NONE;

        animation->reflection = reflection;
    }
}

void PlayerDebugDraw(const Scene* scene, usize entity)
{
    static const u64 dependencies = TAG_PLAYER | TAG_POSITION | TAG_DIMENSION;

    if (!SceneEntityHasDependencies(scene, entity, dependencies))
    {
        return;
    }

    const CPosition* position = &scene->components.positions[entity];
    const CDimension* dimension = &scene->components.dimensions[entity];

    const Rectangle aabb = (Rectangle)
    {
        .x = position->value.x,
        .y = position->value.y,
        .width = dimension->width,
        .height = dimension->height
    };

    // Draw general-purpose aabb.
    {
        const Color color = P8_RED;
        DrawRectangleRec(aabb, ColorAlpha(color, 0.75));
        DrawRectangleLinesEx(aabb, 2, color);
    }
}
