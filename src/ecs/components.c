#include "components.h"

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

Component ComponentCreateCAnimation(CAnimation value)
{
    return (Component)
    {
        .tag = TAG_ANIMATION,
        .animation = value,
    };
}
