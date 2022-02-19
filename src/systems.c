#include "components.h"
#include "context.h"
#include "raymath.h"
#include "systems.h"

#define REQUIRE_DEPS(dependencies) if ((components->tags[entity] & (dependencies)) != (dependencies)) return

void SDummyUpdate(Components* components, usize entity)
{
    REQUIRE_DEPS(tagPosition | tagDimension);

    Vector2 tmp = components->positions[entity].value;
    Vector2 position = Vector2Add(tmp, Vector2Create(64 * CTX_DT, 0));

    components->positions[entity].value = position;
}

void SDummyDraw(Components* components, usize entity)
{
    REQUIRE_DEPS(tagPosition | tagDimension | tagColor);

    CPosition position = components->positions[entity];
    CDimension dimension = components->dimensions[entity];
    CColor color = components->colors[entity];

    Rectangle rectangle = (Rectangle)
    {
        .x = position.value.x,
        .y = position.value.y,
        .width = dimension.width,
        .height = dimension.height
    };

    DrawRectangleRec(rectangle, color.value);
}
