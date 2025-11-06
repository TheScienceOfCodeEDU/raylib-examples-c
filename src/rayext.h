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
    DrawRectangleRec(rect, color);
    DrawRectangleLinesEx(rect, 1.0, color);
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
    float h = fminf(a.y + a.height, b.y + b.height) - y;
    if (w <= 0 || h <= 0)
        return (Rectangle){0, 0, 0, 0};
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

Vector2 AddVector2(Vector2 vector, float dx, float dy)
{
    Vector2 result = {
        vector.x + dx,
        vector.y + dy
    };
    return result;
}

Rectangle RectFromVector2(Vector2 position, float w, float h)
{
    Rectangle result = {
        position.x,
        position.y,
        w,
        h
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


static inline int   IntMax(int a, int b)   { return (a > b) ? a : b; }
static inline int   IntMin(int a, int b)   { return (a < b) ? a : b; }

static inline float FloatMax(float a, float b) { return (a > b) ? a : b; }
static inline float FloatMin(float a, float b) { return (a < b) ? a : b; }

static inline int   IntAbs(int value) { return (value < 0) ? -value : value; }
static inline float FloatAbs(float value) { return (value < 0.0f) ? -value : value; }

// Round up (always to the next integer if not exact)
static inline int FloatCeil(float value)
{
    int i = (int)value;
    return (value > (float)i) ? (i + 1) : i;
}

// Round down (always to the lower integer if not exact)
static inline int FloatFloor(float value)
{
    int i = (int)value;
    return (value < (float)i) ? (i - 1) : i;
}


void DrawTextureFullScreenKeep(Texture2D tex, Color tint)
{
    int screenW = GetScreenWidth();
    int screenH = GetScreenHeight();

    // 🔹 Escala entera máxima (pixel-perfect sin barra)
    int scaleX = screenW / tex.width;
    int scaleY = screenH / tex.height;
    int scale  = (scaleX < scaleY ? scaleX : scaleY);
    if (scale < 1) scale = 1;

    int outW = tex.width  * scale;
    int outH = tex.height * scale;

    // 🔹 Coordenadas exactas centradas en enteros
    int ox = (screenW - outW) / 2;
    int oy = (screenH - outH) / 2;

    // 🔹 Fuente normal (sin invertir Y)
    Rectangle src = {0, 0, (float)tex.width, (float)tex.height};
    Rectangle dst = {(float)ox, (float)oy, (float)outW, (float)outH};

    // 🔹 Dibujo exacto
    DrawTexturePro(tex, src, dst, (Vector2){0, 0}, 0.0f, tint);
}

void DrawTextureFullScreen(Texture2D tex, Color tint)
{
    int screenW = GetScreenWidth();
    int screenH = GetScreenHeight();

    // Evita sangrados de borde
    SetTextureFilter(tex, TEXTURE_FILTER_POINT);
    SetTextureWrap(tex, TEXTURE_WRAP_CLAMP);

    float screenAspect = (float)screenW / (float)screenH;
    float texAspect    = (float)tex.width / (float)tex.height;

    // Destino: ocupa toda la pantalla
    Rectangle dst = { 0, 0, (float)screenW, (float)screenH };

    // Fuente: recorte centrado (cover)
    Rectangle src;
    if (screenAspect > texAspect) {
        // Pantalla más ancha → recorto altura
        float neededH = (float)tex.width / screenAspect;   // h que corresponde para no tener barras
        float y = ((float)tex.height - neededH) * 0.5f;    // centro
        // Redondeo a enteros para evitar 1px fantasma
        int iy = (int)(y + 0.5f);
        int ih = (int)(neededH + 0.5f);
        src = (Rectangle){ 0.0f, (float)iy, (float)tex.width, (float)ih };
    } else {
        // Pantalla más alta → recorto ancho
        float neededW = (float)tex.height * screenAspect;
        float x = ((float)tex.width - neededW) * 0.5f;
        int ix = (int)(x + 0.5f);
        int iw = (int)(neededW + 0.5f);
        src = (Rectangle){ (float)ix, 0.0f, (float)iw, (float)tex.height };
    }

    // OJO: no invertimos Y (src.height positivo) → sale al derecho
    DrawTexturePro(tex, src, dst, (Vector2){0,0}, 0.0f, tint);
}

int StringSize(const char *str) {
    int len = 0;
    while (str[len] != '\0') {
        len++;
    }
    return len;
}