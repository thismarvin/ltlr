#include "../events.h"
#include "common.h"
#include "player.h"
#include <math.h>
#include <raymath.h>

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

static void PlayerSpawnImpactParticles(Scene* scene, const usize entity, const f32 groundY)
{
    assert(SceneEntityHasDependencies(scene, entity, TAG_POSITION | TAG_DIMENSION | TAG_KINETIC));

    const CPosition* position = &scene->components.positions[entity];
    const CDimension* dimension = &scene->components.dimensions[entity];
    const CKinetic* kinetic = &scene->components.kinetics[entity];

    const usize spawnCount = GetRandomValue(20, 40);
    const f32 angleIncrement = (DEG2RAD * 25) / (spawnCount * 0.5);
    const f32 leftMultiplier = kinetic->velocity.x == 0 ? 0.5 : kinetic->velocity.x < 0 ? 0.25 : 0.75;
    const f32 rightMultiplier = 1 - leftMultiplier;
    const f32 spread = dimension->width * 0.25;

    const f32 anchorOffset = dimension->width * 0.3;
    const Vector2 anchor = (Vector2)
    {
        .x = position->value.x + dimension->width * 0.5,
        .y = groundY,
    };
    const Vector2 leftAnchor = (Vector2)
    {
        .x = anchor.x - anchorOffset,
        .y = anchor.y,
    };
    const Vector2 rightAnchor = (Vector2)
    {
        .x = anchor.x + anchorOffset,
        .y = anchor.y,
    };

    static const f32 minimumRadius = 1;
    static const usize radiusVariance = 4;
    static const f32 minimumSpeed = 10.0f;
    static const usize speedVariance = 30;
    static const f32 gravity = 9.8f;

    // Left pocket.
    for (usize i = 0; i < spawnCount * leftMultiplier; ++i)
    {
        const f32 radius = minimumRadius + GetRandomValue(0, radiusVariance);
        const Vector2 cloudPosition = (Vector2)
        {
            .x = leftAnchor.x - GetRandomValue(0, spread),
            .y = leftAnchor.y - radius * 2,
        };
        const f32 rotation = PI + angleIncrement * i;
        const Vector2 direction = (Vector2)
        {
            .x = cosf(rotation),
            .y = sinf(rotation),
        };
        const f32 speed = minimumSpeed + GetRandomValue(0, speedVariance);
        const Vector2 vo = Vector2Scale(direction, speed);
        const Vector2 ao = (Vector2)
        {
            .x = direction.x * -speed * 0.5,
            .y = direction.y * -speed * 0.5 + gravity,
        };

        SceneDeferAddEntity(scene, CloudParticleCreate(cloudPosition, radius, vo, ao));
    }

    // Right pocket.
    for (usize i = 0; i < spawnCount * rightMultiplier; ++i)
    {
        const f32 radius = minimumRadius + GetRandomValue(0, radiusVariance);
        const Vector2 cloudPosition = (Vector2)
        {
            .x = rightAnchor.x - GetRandomValue(0, spread),
            .y = rightAnchor.y - radius * 2,
        };
        const f32 rotation = 0 - angleIncrement * i;
        const Vector2 direction = (Vector2)
        {
            .x = cosf(rotation),
            .y = sinf(rotation),
        };
        const f32 speed = minimumSpeed + GetRandomValue(0, speedVariance);
        const Vector2 vo = Vector2Scale(direction, speed);
        const Vector2 ao = (Vector2)
        {
            .x = direction.x * -speed * 0.5,
            .y = direction.y * -speed * 0.5 + gravity,
        };

        SceneDeferAddEntity(scene, CloudParticleCreate(cloudPosition, radius, vo, ao));
    }
}

static void PlayerOnDamage(const OnDamageParams* params)
{
    assert(ENTITY_HAS_DEPS(params->entity, TAG_PLAYER | TAG_MORTAL));

    CPlayer* player = GET_COMPONENT(player, params->entity);
    CMortal* mortal = GET_COMPONENT(mortal, params->entity);

    const CDamage* otherDamage = GET_COMPONENT(otherDamage, params->otherEntity);

    if (!PlayerIsVulnerable(player))
    {
        return;
    }

    mortal->hp -= otherDamage->value;
    player->invulnerableTimer = 0;
}

static void PlayerOnCollision(const OnCollisionParams* params)
{
    assert(ENTITY_HAS_DEPS(params->entity, TAG_PLAYER | TAG_MORTAL));

    const CMortal* mortal = GET_COMPONENT(mortal, params->entity);

    // Collision specific logic that will not resolve the player.
    {
        if (ENTITY_HAS_DEPS(params->otherEntity, TAG_WALKER | TAG_DAMAGE))
        {
            OnDamageParams onDamageParams = (OnDamageParams)
            {
                .scene = params->scene,
                .entity = params->entity,
                .otherEntity = params->otherEntity,
            };

            mortal->onDamage(&onDamageParams);

            return;
        }
    }
}

static OnResolutionResult PlayerOnResolution(const OnResolutionParams* params)
{
    assert(ENTITY_HAS_DEPS(params->entity, TAG_PLAYER | TAG_POSITION | TAG_KINETIC));

    CPlayer* player = GET_COMPONENT(player, params->entity);
    const CPosition* position = GET_COMPONENT(position, params->entity);
    CKinetic* kinetic = GET_COMPONENT(kinetic, params->entity);

    // Collision specific logic that will not resolve the player.
    {
        if (ENTITY_HAS_DEPS(params->otherEntity, TAG_WALKER))
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
    Rectangle resolvedAabb = ApplyResolutionPerfectly(params->aabb, params->otherAabb,
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

EntityBuilder PlayerCreate(const f32 x, const f32 y)
{
    Deque components = DEQUE_OF(Component);

    const u64 tags =
        TAG_NONE
        | TAG_POSITION
        | TAG_DIMENSION
        | TAG_COLOR
        | TAG_SPRITE
        | TAG_KINETIC
        | TAG_SMOOTH
        | TAG_COLLIDER
        | TAG_PLAYER
        | TAG_MORTAL;

    Vector2 position = Vector2Create(x, y);

    ADD_COMPONENT(CPosition, ((CPosition)
    {
        .value = position,
    }));

    ADD_COMPONENT(CDimension, ((CDimension)
    {
        .width = 15,
        .height = 35,
    }));

    ADD_COMPONENT(CColor, ((CColor)
    {
        .value = COLOR_WHITE,
    }));

    ADD_COMPONENT(CSprite, ((CSprite)
    {
        .source = (Rectangle) { 16, 0, 32, 48 },
        .offset = Vector2Create(-8, -13),
        .mirroring = FLIP_NONE,
    }));

    ADD_COMPONENT(CKinetic, ((CKinetic)
    {
        .velocity = VECTOR2_ZERO,
        .acceleration = VECTOR2_ZERO,
    }));

    ADD_COMPONENT(CSmooth, ((CSmooth)
    {
        .previous = position,
    }));

    ADD_COMPONENT(CCollider, ((CCollider)
    {
        .resolutionSchema = RESOLVE_NONE,
        .layer = LAYER_NONE,
        .mask = LAYER_TERRAIN | LAYER_LETHAL,
        .onCollision = PlayerOnCollision,
        .onResolution = PlayerOnResolution,
    }));

    ADD_COMPONENT(CMortal, ((CMortal)
    {
        .hp = 2,
        .onDamage = PlayerOnDamage,
    }));

    f32 coyoteDuration = CTX_DT * 6;
    f32 invulnerableDuration = 1.5f;

    ADD_COMPONENT(CPlayer, ((CPlayer)
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
    }));

    return (EntityBuilder)
    {
        .tags = tags,
        .components = components,
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

    const f32 vf = 0;
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

void PlayerInputUpdate(Scene* scene, const usize entity)
{
    const u64 dependencies = TAG_PLAYER | TAG_POSITION | TAG_DIMENSION | TAG_KINETIC;

    if (!SceneEntityHasDependencies(scene, entity, dependencies))
    {
        return;
    }

    CPlayer* player = SCENE_GET_COMPONENT_PTR(scene, player, entity);
    const CPosition* position = SCENE_GET_COMPONENT_PTR(scene, position, entity);
    const CDimension* dimension = SCENE_GET_COMPONENT_PTR(scene, dimension, entity);
    CKinetic* kinetic = SCENE_GET_COMPONENT_PTR(scene, kinetic, entity);

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
    const u64 dependencies = TAG_PLAYER | TAG_POSITION | TAG_DIMENSION | TAG_KINETIC;

    if (!SceneEntityHasDependencies(scene, entity, dependencies))
    {
        return;
    }

    CPlayer* player = SCENE_GET_COMPONENT_PTR(scene, player, entity);
    CPosition* position = SCENE_GET_COMPONENT_PTR(scene, position, entity);
    const CDimension* dimension = SCENE_GET_COMPONENT_PTR(scene, dimension, entity);
    CKinetic* kinetic = SCENE_GET_COMPONENT_PTR(scene, kinetic, entity);

    // General purpose player specific collision logic.
    {
        // Keep the player's x within the scene's bounds.
        if (position->value.x < scene->bounds.x)
        {
            position->value.x = scene->bounds.x;
            PlayerStandstill(player, kinetic);
        }
        else if (position->value.x + dimension->width > RectangleRight(scene->bounds))
        {
            position->value.x = RectangleRight(scene->bounds) - dimension->width;
            PlayerStandstill(player, kinetic);
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
    CPlayer* player = SCENE_GET_COMPONENT_PTR(scene, player, entity);

    player->invulnerableTimer += CTX_DT;

    const u32 totalFlashes = 5;
    f32 timeSlice = player->invulnerableDuration / (totalFlashes * 2.0f);

    if (!player->dead && !PlayerIsVulnerable(player))
    {
        u32 passedSlices = (u32)(player->invulnerableTimer / timeSlice);

        if (passedSlices % 2 == 0)
        {
            SceneDeferDisableComponent(scene, entity, TAG_SPRITE);
        }
        else
        {
            SceneDeferEnableComponent(scene, entity, TAG_SPRITE);
        }
    }
    else
    {
        // This is a pre-caution to make sure the last state isn't off.
        SceneDeferEnableComponent(scene, entity, TAG_SPRITE);
    }
}

void PlayerMortalUpdate(Scene* scene, const usize entity)
{
    u64 dependencies = TAG_PLAYER | TAG_MORTAL | TAG_POSITION | TAG_KINETIC;

    if (!SceneEntityHasDependencies(scene, entity, dependencies))
    {
        return;
    }

    CPlayer* player = SCENE_GET_COMPONENT_PTR(scene, player, entity);
    const CMortal* mortal = SCENE_GET_COMPONENT_PTR(scene, mortal, entity);
    const CPosition* position = SCENE_GET_COMPONENT_PTR(scene, position, entity);
    CKinetic* kinetic = SCENE_GET_COMPONENT_PTR(scene, kinetic, entity);

    if (position->value.y > CTX_VIEWPORT_HEIGHT * 2)
    {
        // TODO(thismarvin): Defer resetting the Scene somehow...
        SceneReset(scene);

        return;
    }

    if (!player->dead && mortal->hp <= 0)
    {
        player->dead = true;

        SceneDeferDisableComponent(scene, entity, TAG_COLLIDER);

        kinetic->velocity = (Vector2)
        {
            .x = kinetic->velocity.x,
            .y = -250,
        };
    }

    PlayerFlashingLogic(scene, entity);
}
