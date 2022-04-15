#include "components.h"

const u64 tagNone = (u64)0;
const u64 tagPosition = (u64)1 << 0;
const u64 tagDimension = (u64)1 << 1;
const u64 tagColor = (u64)1 << 2;
const u64 tagSprite = (u64)1 << 3;
const u64 tagKinetic = (u64)1 << 4;
const u64 tagSmooth = (u64)1 << 5;
const u64 tagPlayer = (u64)1 << 6;
const u64 tagCollider = (u64)1 << 7;
const u64 tagWalker = (u64)1 << 8;
const u64 tagMortal = (u64)1 << 9;
const u64 tagDamage = (u64)1 << 10;
const u64 tagFleeting = (u64)1 << 11;

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
        .tag = tagPosition,
        .position = value,
    };
}

Component ComponentCreateCDimension(CDimension value)
{
    return (Component)
    {
        .tag = tagDimension,
        .dimension = value,
    };
}

Component ComponentCreateCColor(CColor value)
{
    return (Component)
    {
        .tag = tagColor,
        .color = value,
    };
}

Component ComponentCreateCSprite(CSprite value)
{
    return (Component)
    {
        .tag = tagSprite,
        .sprite = value,
    };
}

Component ComponentCreateCKinetic(CKinetic value)
{
    return (Component)
    {
        .tag = tagKinetic,
        .kinetic = value,
    };
}

Component ComponentCreateCSmooth(CSmooth value)
{
    return (Component)
    {
        .tag = tagSmooth,
        .smooth = value,
    };
}

Component ComponentCreateCCollider(CCollider value)
{
    return (Component)
    {
        .tag = tagCollider,
        .collider = value,
    };
}

Component ComponentCreateCWalker(CWalker value)
{
    return (Component)
    {
        .tag = tagWalker,
    };
}

Component ComponentCreateCPlayer(CPlayer value)
{
    return (Component)
    {
        .tag = tagPlayer,
        .player = value,
    };
}

Component ComponentCreateCMortal(CMortal value)
{
    return (Component)
    {
        .tag = tagMortal,
        .mortal = value,
    };
}

Component ComponentCreateCDamage(CDamage value)
{
    return (Component)
    {
        .tag = tagDamage,
        .damage = value,
    };
}

Component ComponentCreateCFleeting(CFleeting value)
{
    return (Component)
    {
        .tag = tagFleeting,
        .fleeting = value,
    };
}
