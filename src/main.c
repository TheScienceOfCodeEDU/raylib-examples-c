#include "raylib.h"

void DrawButton(bool status, float scale) 
{
    int paddingTop = 8;
    int paddingLeft = 12;

    int fontSize = 20 ;    

    Color bg_color = status ? (Color) { 116, 100, 67, 255 } : (Color) { 80, 67, 48, 255 };

    Color tx_color = status ? (Color) { 238, 208, 147, 255 } : (Color) { 171, 158, 127, 255 };

    Color lightBorder = { 33, 33, 33, 200 };
    Color darkBorder = { 118, 118, 118, 200 };

    Vector2 text_shape = MeasureTextEx(GetFontDefault(), "Hello raylib", fontSize, 1.0);

    float border = 2 * scale;
    Rectangle button = { 
        0, 0, 250, text_shape.y + paddingTop * 2
    };
    button.x *= scale;
    button.y *= scale;
    button.width *= scale;
    button.height *= scale;

    fontSize *= scale;
    paddingTop *= scale;
    paddingLeft *= scale;    

    // Draw the button background
    DrawRectangleRec(button, bg_color);

    // Draw top border (horizontal line)    
    DrawRectangle(button.x, button.y, button.width, border, darkBorder);

    // Draw left border (vertical line)
    DrawRectangle(button.x, button.y, border, button.height, darkBorder);

    // Draw bottom border (horizontal line)
    DrawRectangle(button.x, button.y + button.height - border, button.width, border, lightBorder);

    // Draw right border (vertical line)
    DrawRectangle(button.x + button.width - border, button.y, border, button.height, lightBorder);

    DrawText("Hello raylib", button.x + paddingLeft, button.y + paddingTop, fontSize, tx_color);
    DrawPixel(0, 0, RED);
}

#define CHARACTERS              4
#define CHARACTER_MAX_SPEED     4

struct  {
    Rectangle Shape;
    Color Color;
} typedef Character;

int main(void) {
    const int screenWidth = 800;
    const int screenHeight = 600;
    InitWindow(screenWidth, screenHeight, TextFormat("Raylib Movement - %s", GetWorkingDirectory()));
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

        static float ui_zoom = 1.0f;
        if (IsKeyPressed(KEY_KP_ADD) || IsKeyPressed(KEY_EQUAL)) ui_zoom += 0.25;
        if (IsKeyPressed(KEY_KP_SUBTRACT) || IsKeyPressed(KEY_MINUS)) ui_zoom -= 0.25;


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

            DrawButton(IsMouseButtonDown(MOUSE_LEFT_BUTTON), ui_zoom);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}