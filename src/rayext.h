#ifndef UNITY_BUILD
 #include "raylib.h"
 #include "raymath.h"
#endif


Rectangle LimitRect(Rectangle shape, Rectangle limits) {
    // Restrict size to not exceed limits
    shape.width = fminf(shape.width, limits.width);
    shape.height = fminf(shape.height, limits.height);

    // Restrict position to not go outside left/top boundaries
    shape.x = fmaxf(shape.x, limits.x);
    shape.y = fmaxf(shape.y, limits.y);

    // Restrict position to not go outside right/bottom boundaries
    shape.x = fminf(shape.x, limits.x + limits.width - shape.width);
    shape.y = fminf(shape.y, limits.y + limits.height - shape.height);

    return shape;
}

Vector2 LimitVector2Rect(Vector2 point, Rectangle limits) {
    // Restrict position to not go outside left/top boundaries
    point.x = fmaxf(point.x, limits.x);
    point.y = fmaxf(point.y, limits.y);

    // Restrict position to not go outside right/bottom boundaries
    point.x = fminf(point.x, limits.x + limits.width);
    point.y = fminf(point.y, limits.y + limits.height);

    return point;
}

void DrawDebugRect(Rectangle rect, Color color)
{
    DrawRectangleRec(rect, ColorAlpha(color, 0.25));
    DrawRectangleLinesEx(rect, 1.0, ColorAlpha(color, 0.65));
}

Rectangle RelativeToRect(Rectangle rectangle, Rectangle relativeTo)
{
    Rectangle result = {
        rectangle.x + relativeTo.x,
        rectangle.y + relativeTo.y,
        rectangle.width > relativeTo.width ? relativeTo.width : rectangle.width,
        rectangle.height > relativeTo.height ? relativeTo.height : rectangle.height
    };    
    return result;
}

bool IsKeyEnterPressed()
{
    return IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER);
}

bool CheckCollisionPointRecWithMargin(Vector2 point, Rectangle rect, float margin) {
    Rectangle inner = {
        rect.x + margin,
        rect.y + margin, 
        rect.width - 2 * margin,
        rect.height - 2 * margin
    };

    return CheckCollisionPointRec(point, inner);
}
