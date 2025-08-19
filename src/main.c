#include "raylib.h"

struct {
    Texture2D New;
    Texture2D Open;
    Texture2D Save;
} typedef GUI_Icons;

GUI_Icons GUI_LoadIcons()
{
    GUI_Icons icons = {
        LoadTexture("ico/new.png"),
        LoadTexture("ico/open.png"),
        LoadTexture("ico/save.png")
    };
    return icons;
}

enum {
    GUI_Status_Default,
    GUI_Status_Focus,
    GUI_Status_Click
} typedef GUI_ElementStatus;

struct {
    Color bg_color_0;
    Color bg_color_1;
    Color bg_color_2;
    Color color_0;
    Color color_1;
    Color color_2;
    Color b_color_0;
    Color b_color_1;
    Vector2 padding;
    float border;
    float font_size;
    float font_spacing;
} typedef GUI_Theme;

GUI_Theme GUI_MakeDefaultTheme(int opacity)
{
    GUI_Theme theme = {        
        (Color) { 80, 67, 48, opacity },
        (Color) { 116, 100, 67, opacity },
        (Color) { 58, 49, 35, opacity },

        (Color) { 171, 158, 127, 255 },
        (Color) { 238, 208, 147, 255 },
        (Color) { 253, 250, 85, 255 },

        (Color) { 33, 33, 33, 255 },
        (Color) { 118, 118, 118, 255 },

        (Vector2){ 8, 8 },
        2.0f,
        20.0f,
        1.0f
    };

    return theme;
}

float GUI_CalcDefaultHeight(float font_size)
{
    Vector2 textShape = MeasureTextEx(GetFontDefault(), "Hello raylib", font_size, 1.0);
    return textShape.y;
}

float GUI_CalcDefaultIconSize(float font_size, float scale)
{
    return GUI_CalcDefaultHeight(font_size) * scale;
}

struct {
    GUI_Theme theme;
    GUI_Icons icons;
    float scale;
    Font font;
} typedef GUI_State;

GUI_State GUI_MakeDefaultState(float opacity)
{
    GUI_State state = {
        GUI_MakeDefaultTheme(255),
        GUI_LoadIcons(),
        1.0f,
        LoadFontEx("fnt/VT323.ttf", 64.0f, 0, 0)
    };

    SetTextureFilter(state.font.texture, TEXTURE_FILTER_POINT);
    return state;
}

void GUI_DrawButton(char* text, Rectangle shape,  GUI_ElementStatus status, GUI_Theme theme, Font font, float scale, bool hasIcon) 
{
    Color bg_color = status == GUI_Status_Default ? theme.bg_color_0 : theme.bg_color_1;
    Color tx_color = status == GUI_Status_Default ? theme.color_0 : theme.color_1;

    Color b_light = status == GUI_Status_Click ? theme.b_color_1 : theme.b_color_0;
    Color b_dark = status == GUI_Status_Click ? theme.b_color_0 : theme.b_color_1;

    // Draw the button background
    DrawRectangleRec(shape, bg_color);

    // Draw top border (horizontal line)    
    DrawRectangle(shape.x, shape.y, shape.width, theme.border, b_dark);

    // Draw left border (vertical line)
    DrawRectangle(shape.x, shape.y, theme.border, shape.height, b_dark);

    // Draw bottom border (horizontal line)
    DrawRectangle(shape.x, shape.y + shape.height - theme.border, shape.width, theme.border, b_light);

    // Draw right border (vertical line)
    DrawRectangle(shape.x + shape.width - theme.border, shape.y, theme.border, shape.height, b_light);

    // Calc text padding
    float icon_size = hasIcon ? GUI_CalcDefaultIconSize(theme.font_size, scale) : 0;    
    //DrawText(text, shape.x + icon_size + theme.padding.x * 2, shape.y + theme.padding.y, theme.font_size * scale, tx_color);
    DrawTextEx(font, text, 
        (Vector2){shape.x + icon_size + theme.padding.x * 2, shape.y + theme.padding.y}, 
        theme.font_size * scale, theme.font_spacing, tx_color);
    DrawPixel(0, 0, RED);
}

void GUI_Icon(Texture2D* texture2d, Vector2 position, float font_size, float scale, Color tint)
{
    float height = GUI_CalcDefaultHeight(font_size);
    scale *= height / texture2d->height;
    DrawTextureEx(*texture2d, position, 0, scale, tint);
}

bool GUI_Button(char* text, Rectangle shape, GUI_State* gui, Texture2D* icon)
{
    GUI_Theme theme = gui->theme;
    Vector2 mouse = GetMousePosition();
    GUI_ElementStatus status = GUI_Status_Default;
    if (CheckCollisionPointRec(mouse, shape)) {
        if (!IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            status = GUI_Status_Focus;
        } else {
            status = GUI_Status_Click;
        }
    }
    
    bool hasIcon = icon != 0;
    GUI_DrawButton(text, shape, status, theme, gui->font, gui->scale, hasIcon);

    GUI_Icon(icon, (Vector2) { shape.x + theme.padding.x, shape.y + theme.padding.y }, theme.font_size, gui->scale, WHITE);

    return IsMouseButtonReleased(MOUSE_BUTTON_LEFT);
}

void GUI_TopBar(GUI_State* gui)
{
    int buttons = 3;
    float screen_w = GetScreenWidth();
    float button_w = screen_w / buttons;

    float button_h = GUI_CalcDefaultHeight(gui->theme.font_size) * gui->scale + gui->theme.padding.y * 2;


    GUI_Button("New game", (Rectangle) { button_w * 0, 0, button_w, button_h }, gui, &gui->icons.New);
    GUI_Button("Load", (Rectangle) { button_w * 1, 0, button_w, button_h }, gui, &gui->icons.Open);
    GUI_Button("Save", (Rectangle) { button_w * 2, 0, button_w, button_h }, gui, &gui->icons.Save);
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
    GUI_State gui = GUI_MakeDefaultState(255);

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

        if (IsKeyPressed(KEY_KP_ADD) || IsKeyPressed(KEY_EQUAL)) gui.scale += 0.5;
        if (IsKeyPressed(KEY_KP_SUBTRACT) || IsKeyPressed(KEY_MINUS)) gui.scale -= 0.5;

        static float ui_opacity = 250.0;

        // 
        // RENDER
        //

        // UI Buffer
        BeginTextureMode(buffer);
            ClearBackground(BLANK);
            GUI_TopBar(&gui);
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