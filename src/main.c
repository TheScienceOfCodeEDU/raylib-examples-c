#include "raylib.h"

#define CHARACTERS              4
#define CHARACTER_MAX_SPEED     4

struct  {
    Rectangle Shape;
    Color Color;
} typedef Character;

int main(void) {
    const int screenWidth = 800;
    const int screenHeight = 600;
    InitWindow(screenWidth, screenHeight, "Raylib Movement");
    SetTargetFPS(60);

    int current_character = 0;

    Character characters[CHARACTERS] = {
        { 0, 0, 10, 20, RED},
        { 10, 30, 10, 20, BLUE},
        { 50, 60, 10, 20, GREEN},
        { 80, 60, 10, 20, ORANGE}
    };

    Camera2D camera = { 0 };
    camera.target = (Vector2){ 0, 0 };
    camera.offset = (Vector2){ screenWidth/2.0f, screenHeight/2.0f };
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;

    SetTargetFPS(60); 

    while (!WindowShouldClose()) {
        //
        // UPDATE
        //
        
        // Toggle player
        if (IsKeyPressed(KEY_TAB)) current_character = (current_character + 1) % CHARACTERS;

        // Update character
        Character *player = &characters[current_character];
        if (IsKeyDown(KEY_LEFT))    player->Shape.x -= CHARACTER_MAX_SPEED;
        if (IsKeyDown(KEY_RIGHT))   player->Shape.x += CHARACTER_MAX_SPEED;
        if (IsKeyDown(KEY_UP))      player->Shape.y -= CHARACTER_MAX_SPEED;
        if (IsKeyDown(KEY_DOWN))    player->Shape.y += CHARACTER_MAX_SPEED;
        
        // Update camera
        camera.target = (Vector2){ player->Shape.x, player->Shape.y };
        camera.zoom += ((float)GetMouseWheelMove()*0.1f);
        if (camera.zoom > 3.0f) camera.zoom = 3.0f;
        else if (camera.zoom < 0.1f) camera.zoom = 0.1f;

        // 
        // DRAW
        //
        BeginDrawing();
            ClearBackground(RAYWHITE);
            BeginMode2D(camera);

            for (int i = 0; i < CHARACTERS; ++i ) {
                Character *c = &characters[i];
                DrawRectangleRec(c->Shape, c->Color);
            }
            
            EndMode2D();

        EndDrawing();
    }

    CloseWindow();
    return 0;
}