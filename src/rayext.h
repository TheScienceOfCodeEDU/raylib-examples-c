#ifndef UNITY_BUILD
 #include "raylib.h"
 #include "raymath.h"
 #include "rlgl.h"
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
    DrawRectangleRec(rect, ColorAlpha(color, 0.75));
    DrawRectangleLinesEx(rect, 1.0, ColorAlpha(color, 0.95));
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

static inline Rectangle RectIntersection(Rectangle a, Rectangle b)
{
    float x = fmaxf(a.x, b.x);
    float y = fmaxf(a.y, b.y);
    float w = fminf(a.x + a.width,  b.x + b.width)  - x;
    float h = fminf(a.y + a.height, a.y + b.height) - y;
    if (w <= 0 || h <= 0)
        return (Rectangle){0,0,0,0};
    else
        return (Rectangle){x, y, w, h};
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

// Returns a Rectangle representing the full source area of the given Texture2D
Rectangle GetSourceRec(Texture2D texture)
{
    Rectangle sourceRec = { 0.0f, 0.0f, (float)texture.width, (float)texture.height };
    return sourceRec;
}

Rectangle FlipYRec(Rectangle rectangle)
{
    Rectangle result = {
        rectangle.x,
        rectangle.y,
        rectangle.width,
        -rectangle.height
    };
    return result;
}

// TODO@dc: rename
Rectangle MoveAndExtendXY(Rectangle rectangle, float dx, float dy)
{
    Rectangle result = {
        rectangle.x - dx,
        rectangle.y - dy,
        rectangle.width + dx,
        rectangle.height + dy
    };
    return result;
}

Rectangle MoveRect(Rectangle rect, Vector2 offset)
{
    Rectangle result = {
        rect.x + offset.x,
        rect.y + offset.y,
        rect.width,
        rect.height
    };
    return result;
}

Rectangle AddRect(Rectangle rectangle, float dx, float dy, float dw, float dh)
{
    Rectangle result = {
        rectangle.x + dx,
        rectangle.y + dy,
        rectangle.width + dw,
        rectangle.height + dh
    };
    return result;
}

// Helper function to convert HSV to RGB
Color HSVToRGB(float h, float s, float v)
{
    float c = v * s;
    float x = c * (1 - fabs(fmod(h / 60.0f, 2) - 1));
    float m = v - c;
    float r, g, b;

    if (h >= 0 && h < 60) { r = c; g = x; b = 0; }
    else if (h >= 60 && h < 120) { r = x; g = c; b = 0; }
    else if (h >= 120 && h < 180) { r = 0; g = c; b = x; }
    else if (h >= 180 && h < 240) { r = 0; g = x; b = c; }
    else if (h >= 240 && h < 300) { r = x; g = 0; b = c; }
    else { r = c; g = 0; b = x; }

    return (Color){
        (unsigned char)((r + m) * 255),
        (unsigned char)((g + m) * 255),
        (unsigned char)((b + m) * 255),
        255
    };
}

void BeginScissorModeRect(Rectangle rect) {
    BeginScissorMode((int)rect.x, (int)rect.y, (int)rect.width, (int)rect.height);
}
