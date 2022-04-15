#include "components.h"

const u64 layerNone = (u64)0;
const u64 layerUp = (u64)1 << 0;
const u64 layerRight = (u64)1 << 1;
const u64 layerDown = (u64)1 << 2;
const u64 layerLeft = (u64)1 << 3;
const u64 layerAll = layerUp | layerRight | layerDown | layerLeft;

Component ComponentCreateCPosition(CPosition value)
{
    return (Component)
    {
        .tag = TAG_POSITION,
        .position = value,
    };
}

Component ComponentCreateCDimension(CDimension value)
{
    return (Component)
    {
        .tag = TAG_DIMENSION,
        .dimension = value,
    };
}

Component ComponentCreateCColor(CColor value)
{
    return (Component)
    {
        .tag = TAG_COLOR,
        .color = value,
    };
}

Component ComponentCreateCSprite(CSprite value)
{
    return (Component)
    {
        .tag = TAG_SPRITE,
        .sprite = value,
    };
}

Component ComponentCreateCKinetic(CKinetic value)
{
    return (Component)
    {
        .tag = TAG_KINETIC,
        .kinetic = value,
    };
}

Component ComponentCreateCSmooth(CSmooth value)
{
    return (Component)
    {
        .tag = TAG_SMOOTH,
        .smooth = value,
    };
}

Component ComponentCreateCCollider(CCollider value)
{
    return (Component)
    {
        .tag = TAG_COLLIDER,
        .collider = value,
    };
}

Component ComponentCreateCWalker(CWalker value)
{
    return (Component)
    {
        .tag = TAG_WALKER,
    };
}

Component ComponentCreateCPlayer(CPlayer value)
{
    return (Component)
    {
        .tag = TAG_PLAYER,
        .player = value,
    };
}

Component ComponentCreateCMortal(CMortal value)
{
    return (Component)
    {
        .tag = TAG_MORTAL,
        .mortal = value,
    };
}

Component ComponentCreateCDamage(CDamage value)
{
    return (Component)
    {
        .tag = TAG_DAMAGE,
        .damage = value,
    };
}

Component ComponentCreateCFleeting(CFleeting value)
{
    return (Component)
    {
        .tag = TAG_FLEETING,
        .fleeting = value,
    };
}
