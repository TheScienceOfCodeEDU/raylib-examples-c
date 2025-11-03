#ifndef UNITY_BUILD
#include "rayext.h"
#endif

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
