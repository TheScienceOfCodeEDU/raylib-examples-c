#ifndef UNITY_BUILD
#include "rayext.h"
#endif

void DrawCharacter(Rectangle shape, Vector2 movement, float anim_time, Color color)
{
    float h = shape.height;
    float w = shape.width;
    Vector2 base = { shape.x, shape.y };

    float speed = Vector2Length(movement);
    float phase = anim_time * (2.5f + speed * 0.4f);
    float cycle = fmodf(phase, PI * 2.0f);

    // Movimiento vertical del cuerpo (ligero "rebote")
    float body_bob = sinf(cycle * 2.0f) * (h * 0.04f);

    // ----------------------------------------------------------
    // ARTICULACIONES CENTRALES
    // ----------------------------------------------------------
    Vector2 hip   = { base.x + w * 0.5f, base.y + h * 0.6f + body_bob };
    Vector2 neck  = { hip.x, base.y + h * 0.15f + body_bob };
    Vector2 head  = { neck.x, base.y - h * 0.1f + body_bob };

    // ----------------------------------------------------------
    // PIERNAS (movimiento elíptico: x=sin, y=cos)
    // ----------------------------------------------------------
    float legAmpX = w * 0.18f;  // amplitud horizontal
    float legAmpY = h * 0.10f;  // amplitud vertical
    float legOffsetY = h * 0.25f;

    // pierna delantera (fase normal)
    Vector2 knee_front = {
        hip.x + sinf(cycle) * legAmpX * 0.5f,
        hip.y + legOffsetY + cosf(cycle) * legAmpY
    };
    Vector2 foot_front = {
        knee_front.x + sinf(cycle) * legAmpX * 0.7f,
        knee_front.y + h * 0.22f
    };

    // pierna trasera (fase opuesta)
    Vector2 knee_back = {
        hip.x + sinf(cycle + PI) * legAmpX * 0.5f,
        hip.y + legOffsetY + cosf(cycle + PI) * legAmpY
    };
    Vector2 foot_back = {
        knee_back.x + sinf(cycle + PI) * legAmpX * 0.7f,
        knee_back.y + h * 0.22f
    };

    // ----------------------------------------------------------
    // BRAZOS (fase opuesta a las piernas)
    // ----------------------------------------------------------
    float armAmpX = w * 0.25f;
    float armAmpY = h * 0.12f;
    float armOffsetY = h * 0.20f;

    Vector2 shoulder = { neck.x, neck.y + h * 0.05f };

    // brazo delantero
    Vector2 elbow_front = {
        shoulder.x + sinf(cycle + PI) * armAmpX * 0.4f,
        shoulder.y + armOffsetY + cosf(cycle + PI) * armAmpY
    };
    Vector2 hand_front = {
        elbow_front.x + sinf(cycle + PI) * armAmpX * 0.4f,
        elbow_front.y + h * 0.18f
    };

    // brazo trasero
    Vector2 elbow_back = {
        shoulder.x + sinf(cycle) * armAmpX * 0.4f,
        shoulder.y + armOffsetY + cosf(cycle) * armAmpY
    };
    Vector2 hand_back = {
        elbow_back.x + sinf(cycle) * armAmpX * 0.4f,
        elbow_back.y + h * 0.18f
    };

    // ----------------------------------------------------------
    // DIBUJO: líneas (huesos) + puntos (articulaciones)
    // ----------------------------------------------------------
    Color joint = ColorAlpha(color, 0.8);
    Color joint_front = ColorAlpha(color, 0.95);
    Color joint_back = ColorAlpha(color, 0.75);
    Color bone  = (Color){200, 200, 220, 255};

    // cuerpo
    DrawLineV(head, neck, bone);
    DrawLineV(neck, hip, bone);

    // brazos
    DrawLineV(shoulder, elbow_front, bone);
    DrawLineV(elbow_front, hand_front, bone);
    DrawLineV(shoulder, elbow_back, bone);
    DrawLineV(elbow_back, hand_back, bone);

    // piernas
    DrawLineV(hip, knee_front, bone);
    DrawLineV(knee_front, foot_front, bone);
    DrawLineV(hip, knee_back, bone);
    DrawLineV(knee_back, foot_back, bone);

    // puntos visibles
    DrawCircleV(head, 3, joint);
    DrawCircleV(neck, 1, joint);
    DrawCircleV(hip, 1, joint);
    DrawCircleV(knee_front, 1, joint_front);
    DrawCircleV(knee_back, 1, joint_back);
    DrawCircleV(foot_front, 1, joint_front);
    DrawCircleV(foot_back, 1, joint_back);
    DrawCircleV(elbow_front, 1, joint_front);
    DrawCircleV(elbow_back, 1, joint_back);
    DrawCircleV(hand_front, 1, joint_front);
    DrawCircleV(hand_back, 1, joint_back);
}


// Maximum number of points (static array size)
#define MAX_POINTS 50

// Function to generate a Voronoi-like texture using Raylib Math
Texture2D GenerateVoronoiTexture(int width, int height)
{
    // Static arrays for points and colors
    Vector2 points[MAX_POINTS];
    Color colors[MAX_POINTS];

    // Generate random points and colors within hue range
    for (int i = 0; i < MAX_POINTS; i++)
    {
        points[i] = (Vector2){ (float)GetRandomValue(0, width), (float)GetRandomValue(0, height) };
        // Generate colors in HSV space with fixed hue, random saturation (0.5-1.0), and value (0.5-1.0)
        float randomHue = 200 /* HUE */  + GetRandomValue(-30, 30); // Slight variation (±30 degrees) around input hue
        if (randomHue < 0) randomHue += 360;
        if (randomHue >= 360) randomHue -= 360;
        float saturation = GetRandomValue(40, 80) / 100.0f; // 0.5 to 1.0
        float value = GetRandomValue(50, 100) / 100.0f;     // 0.5 to 1.0
        colors[i] = HSVToRGB(randomHue, saturation, value);
    }

    // Initialize RenderTexture2D
    RenderTexture2D target = LoadRenderTexture(width, height);

    // Begin drawing to the texture
    BeginTextureMode(target);
    ClearBackground(BLANK); // Transparent background

    // For each pixel, find the closest point and color accordingly
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            float minDist = 999999.0f;
            int closestPoint = 0;

            // Find the closest point using Raylib Math
            Vector2 pixelPos = (Vector2){ (float)x, (float)y };
            for (int i = 0; i < MAX_POINTS; i++)
            {
                float dist = Vector2Distance(pixelPos, points[i]);
                if (dist < minDist)
                {
                    minDist = dist;
                    closestPoint = i;
                }
            }

            // Draw pixel with the color of the closest point
            DrawPixel(x, y, colors[closestPoint]);
        }
    }

    EndTextureMode();

    // Return the texture
    return target.texture;
}

// Maximum number of rain particles
#define MAX_PARTICLES 512

// Function to generate a rain-like texture
// Function to generate a rain-like texture
void DrawRain(int width, int height, float speed, Color color)
{
    // Static arrays for particle positions, directions, speeds, and angles
    static Vector2 positions[MAX_PARTICLES];
    static Vector2 directions[MAX_PARTICLES];
    static float speeds[MAX_PARTICLES];
    static float angles[MAX_PARTICLES];
    static bool initialized = false;

    // Initialize particles (once)
    if (!initialized)
    {
        for (int i = 0; i < MAX_PARTICLES; i++)
        {
            positions[i] = (Vector2){ (float)GetRandomValue(0, width), (float)GetRandomValue(0, height) };
            // Random direction: mostly down with slight left/right variation (240° to 300°)
            angles[i] = (float)GetRandomValue(240, 300); // 240° (down-left), 270° (down), 300° (down-right)
            directions[i] = (Vector2){ cosf(angles[i] * DEG2RAD), sinf(angles[i] * DEG2RAD) };
            // Random speed variation: 0.5x to 1.5x of input speed
            speeds[i] = speed * GetRandomValue(50, 150) / 100.0f;
        }
        initialized = true;
    }

    // Update particle positions and angles
    for (int i = 0; i < MAX_PARTICLES; i++)
    {
        // Clamp angle to ensure downward movement (280° to 300°)
        if (angles[i] < 280.0f) angles[i] = 280.0f;
        if (angles[i] > 300.0f) angles[i] = 300.0f;
        // Update direction based on new angle
        directions[i] = (Vector2){ cosf(angles[i] * DEG2RAD), sinf(angles[i] * DEG2RAD) };
        // Update position
        positions[i] = Vector2Add(positions[i], Vector2Scale(directions[i], speeds[i]));
        // Wrap around if particle goes out of bounds
        if (positions[i].x < 0) positions[i].x += width;
        if (positions[i].x > width) positions[i].x -= width;
        if (positions[i].y < 0) positions[i].y += height;
        if (positions[i].y > height) positions[i].y -= height;
    }

    // Draw particles as short lines
    for (int i = 0; i < MAX_PARTICLES; i++)
    {
        Vector2 endPos = Vector2Add(positions[i], Vector2Scale(directions[i], 5.0f)); // Line length
        DrawLineV(positions[i], endPos, color);
    }
}
