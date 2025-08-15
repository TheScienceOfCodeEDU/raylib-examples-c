#include "raylib.h"

enum {
    UI_Status_Default,
    UI_Status_Focus,
    UI_Status_Click
} typedef UI_ElementStatus;


float GetButtonHeightDefault(float fontSize)
{
    Vector2 textShape = MeasureTextEx(GetFontDefault(), "Hello raylib", fontSize, 1.0);
    return textShape.y;
}

void DrawButton(Rectangle shape, Vector2 padding, float border, UI_ElementStatus status, float fontSize) 
{
    Color bg_color = status == UI_Status_Default ? (Color) { 116, 100, 67, 255 } : (Color) { 80, 67, 48, 255 };
    Color tx_color = status == UI_Status_Click ? (Color) { 171, 158, 127, 255 } : (Color) { 238, 208, 147, 255 };

    Color lightBorder = status == UI_Status_Click ? (Color) { 118, 118, 118, 255 } : (Color) { 33, 33, 33, 255 };
    Color darkBorder = status == UI_Status_Click ? (Color) { 33, 33, 33, 255 } : (Color) { 118, 118, 118, 255 };      

    // Draw the button background
    DrawRectangleRec(shape, bg_color);

    // Draw top border (horizontal line)    
    DrawRectangle(shape.x, shape.y, shape.width, border, darkBorder);

    // Draw left border (vertical line)
    DrawRectangle(shape.x, shape.y, border, shape.height, darkBorder);

    // Draw bottom border (horizontal line)
    DrawRectangle(shape.x, shape.y + shape.height - border, shape.width, border, lightBorder);

    // Draw right border (vertical line)
    DrawRectangle(shape.x + shape.width - border, shape.y, border, shape.height, lightBorder);

    DrawText("Hello raylib 2", shape.x + padding.x, shape.y + padding.y, fontSize, tx_color);
    DrawPixel(0, 0, RED);
}

void UpdateAndDrawButton(Vector2 position, float scale)
{
    Vector2 padding = { 12, 8 };
    int fontSize = 20 ;

    float height = GetButtonHeightDefault(fontSize);

    float border = 2 * scale;
    Rectangle button = { 
        position.x, position.y, 250, height + padding.y * 2
    };
    button.width *= scale;
    button.height *= scale;

    fontSize *= scale;
    padding.y *= scale;
    padding.x *= scale;

    Vector2 mouse = GetMousePosition();
    UI_ElementStatus status = UI_Status_Default;
    if (CheckCollisionPointRec(mouse, button)) {
        if (!IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            status = UI_Status_Focus;
        } else {
            status = UI_Status_Click;
        }
    }
    
    DrawButton(button, padding, border, status, fontSize);
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

    // Create render texture for the UI
    RenderTexture2D buffer = LoadRenderTexture(screenWidth, screenHeight);

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
        if (IsKeyPressed(KEY_KP_ADD) || IsKeyPressed(KEY_EQUAL)) ui_zoom += 0.5;
        if (IsKeyPressed(KEY_KP_SUBTRACT) || IsKeyPressed(KEY_MINUS)) ui_zoom -= 0.5;

        static float ui_opacity = 250.0;

        // 
        // RENDER
        //

        // UI Buffer
        BeginTextureMode(buffer);
            ClearBackground(BLANK);
            UpdateAndDrawButton((Vector2){ 50, 50 }, ui_zoom);
        EndTextureMode();

        // Draw
        BeginDrawing();
            ClearBackground(RAYWHITE);

            // Game world
            BeginMode2D(camera);
                for (int i = 0; i < CHARACTERS; ++i ) {
                    Character *c = &characters[i];
                    DrawRectangleRec(c->Shape, c->Color);
                }            
            EndMode2D();
            
            // Draw UI Buffer
            Rectangle sourceRec = { 0, 0, (float)buffer.texture.width, -(float)buffer.texture.height };
            DrawTextureRec(buffer.texture, sourceRec, (Vector2){ 0, 0 }, (Color){ 255, 255, 255, ui_opacity});
        EndDrawing();
    }

    CloseWindow();
    return 0;
}