#include "raylib.h"
#include <stdlib.h>

int main(void) {
    InitWindow(800, 600, "Raylib Interactive Polygon with Add/Remove Points");
    SetTargetFPS(60);

    // Initial polygon vertices (in local space)
    Vector2 basePoints[] = {
        {0.0f, 0.0f},   // (0,0)
        {0.0f, 3.0f},   // (0,3)
        {3.0f, 3.0f},   // (3,3)
        {4.0f, 1.5f},   // (4,1.5)
        {3.0f, 0.0f}    // (3,0)
    };
    int pointCount = 5;
    int maxPoints = 100; // Maximum number of points (arbitrary limit)
    Vector2 *points = (Vector2 *)malloc(maxPoints * sizeof(Vector2));
    for (int i = 0; i < pointCount; i++) points[i] = basePoints[i];

    // Interactivity variables
    Vector2 offset = {400.0f, 300.0f}; // Start at window center
    float scale = 100.0f;              // Initial scale factor
    bool filled = true;                // Toggle filled vs outline
    float moveSpeed = 5.0f;            // Pixels per frame for movement
    Vector2 renderPoints[100];         // Array for transformed points

    while (!WindowShouldClose()) {
        // Update scale with mouse wheel
        float wheel = GetMouseWheelMove();
        if (wheel != 0) {
            scale += wheel * 10.0f;
            if (scale < 10.0f) scale = 10.0f; // Minimum scale
        }

        // Move polygon with arrow keys
        if (IsKeyDown(KEY_RIGHT)) offset.x += moveSpeed;
        if (IsKeyDown(KEY_LEFT)) offset.x -= moveSpeed;
        if (IsKeyDown(KEY_UP)) offset.y -= moveSpeed;
        if (IsKeyDown(KEY_DOWN)) offset.y += moveSpeed;

        // Toggle filled/outline with 'F' key
        if (IsKeyPressed(KEY_F)) filled = !filled;

        // Add point with left mouse click
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && pointCount < maxPoints) {
            Vector2 mousePos = GetMousePosition();
            // Convert mouse position to local polygon coordinates
            points[pointCount].x = (mousePos.x - offset.x) / scale;
            points[pointCount].y = (mousePos.y - offset.y) / scale;
            pointCount++;
        }

        // Remove last point with 'R' key
        if (IsKeyPressed(KEY_R) && pointCount > 3) { // Minimum 3 points for a polygon
            pointCount--;
        }

        // Apply scale and offset to points for rendering
        for (int i = 0; i < pointCount; i++) {
            renderPoints[i].x = points[i].x * scale + offset.x;
            renderPoints[i].y = points[i].y * scale + offset.y;
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);

        // Draw polygon (filled or outline)
        if (filled && pointCount >= 3) {
            DrawTriangleFan(renderPoints, pointCount, RED);
        } else {
            DrawLineStrip(renderPoints, pointCount, BLACK);
            if (pointCount >= 3) {
                DrawLineV(renderPoints[pointCount - 1], renderPoints[0], BLACK); // Close polygon
            }
        }

        // Draw instructions
        DrawText("Arrow keys: Move | Mouse wheel: Scale | F: Toggle filled/outline | Left click: Add point | R: Remove point", 10, 10, 20, DARKGRAY);
        DrawText(TextFormat("Points: %d", pointCount), 10, 40, 20, DARKGRAY);

        EndDrawing();
    }

    // Clean up
    free(points);
    CloseWindow();
    return 0;
}