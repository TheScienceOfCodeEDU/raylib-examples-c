#define UNITY_BUILD 1
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
    bool font_custom;
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
        1.0f,
        0
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
    int focus_id;
} typedef GUI_State;

GUI_State GUI_MakeDefaultState(float opacity)
{
    GUI_State state = {
        GUI_MakeDefaultTheme(255),
        GUI_LoadIcons(),
        1.0f,
        LoadFontEx("fnt/VT323.ttf", 64.0f, 0, 0),
        
    };
    
    SetTextureFilter(state.font.texture, TEXTURE_FILTER_POINT);
    return state;
}

void GUI_DrawButton(char* text, Rectangle shape,  GUI_ElementStatus status, GUI_Theme theme, Font font_custom, float scale, bool icon) 
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
    float icon_size = icon ? GUI_CalcDefaultIconSize(theme.font_size, scale) : 0;

    Font font = theme.font_custom ? font_custom : GetFontDefault();
    DrawTextEx(font, text, 
        (Vector2){shape.x + icon_size + theme.padding.x * 2, shape.y + theme.padding.y}, 
        theme.font_size * scale, theme.font_spacing, tx_color);
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

    bool collide = CheckCollisionPointRec(mouse, shape);
    if (collide) {
        if (!IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            status = GUI_Status_Focus;
        } else {
            status = GUI_Status_Click;
        }
    }
    
    bool hasIcon = icon != 0;
    GUI_DrawButton(text, shape, status, theme, gui->font, gui->scale, hasIcon);

    GUI_Icon(icon, (Vector2) { shape.x + theme.padding.x, shape.y + theme.padding.y }, theme.font_size, gui->scale, WHITE);

    return collide && IsMouseButtonReleased(MOUSE_BUTTON_LEFT);
}

void GUI_DrawWindow(char* title, Rectangle shape,  GUI_ElementStatus status, GUI_Theme theme, Font font_custom, float scale, bool icon)
{
    Color bg_color = status == GUI_Status_Default ? theme.bg_color_2 : theme.bg_color_1;
    Color tx_color = theme.color_0;
    
    DrawRectangleRec(shape, bg_color);
    DrawTextEx(font_custom, title,
        (Vector2){shape.x + theme.padding.x, shape.y + theme.padding.y}, 
        theme.font_size * scale, theme.font_spacing, tx_color);
}

void GUI_Window(int id, char* title, Rectangle shape, GUI_State* gui)
{
    GUI_Theme theme = gui->theme;
    Vector2 mouse = GetMousePosition();
    
    bool collide = CheckCollisionPointRec(mouse, shape);
    if (collide && IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
        gui->focus_id = id;
    }

    GUI_ElementStatus status = gui->focus_id == id ? GUI_Status_Focus : GUI_Status_Default;
    GUI_DrawWindow(title, shape, status, theme, gui->font, gui->scale, false);
}

struct {
    bool reset_characters;
    bool add_character;
    bool toggle_character;
    bool move_up;
    bool move_down;
    bool move_left;
    bool move_right;
} typedef PLAYER_Actions;

PLAYER_Actions PLAYER_MakeActions()
{
    PLAYER_Actions actions = { 0 };
    return actions;
}

void GUI_TopBar(GUI_State* gui, PLAYER_Actions* actions)
{
    int buttons = 3;
    float screen_w = GetScreenWidth();
    float button_w = screen_w / buttons;

    float button_h = GUI_CalcDefaultHeight(gui->theme.font_size) * gui->scale + gui->theme.padding.y * 2;

    actions->reset_characters    = GUI_Button("Reset", (Rectangle) { button_w * 0, 0, button_w, button_h }, gui, &gui->icons.New);
    actions->add_character       = GUI_Button("Add", (Rectangle) { button_w * 1, 0, button_w, button_h }, gui, &gui->icons.Open);
    actions->toggle_character    = GUI_Button("Change", (Rectangle) { button_w * 2, 0, button_w, button_h }, gui, &gui->icons.Save);
}

#define CHARACTERS              4
#define CHARACTER_MAX_SPEED     6

struct  {
    Rectangle Shape;
    Color Color;
} typedef Game_Character;

struct {
    int current_character;
    int alive_characters;
    Game_Character characters[CHARACTERS];
} typedef Game_State;

Game_State Game_MakeState()
{
    Game_State state = {
        0,
        2,
        (Game_Character){ 0, 0, 10, 20, RED},
        (Game_Character){ 10, 30, 10, 20, BLUE},
        (Game_Character){ 50, 60, 10, 20, GREEN},
        (Game_Character){ 80, 60, 10, 20, ORANGE},
    };
    return state;
}

void Game_UpdateNextCharacter(Game_State* state)
{
    state->current_character = (state->current_character + 1) % state->alive_characters;
}

void Game_AddCharacter(Game_State* state)
{
    state->alive_characters++;
    if (state->alive_characters > CHARACTERS) state->alive_characters = CHARACTERS;
}

Game_Character* Game_GetCurrentCharacter(Game_State* state)
{
    return &state->characters[state->current_character];
}

int main(void) {
    const int screenWidth = 800;
    const int screenHeight = 600;
    InitWindow(screenWidth, screenHeight, TextFormat("Raylib Movement - %s", GetWorkingDirectory()));
    SetTargetFPS(60);

    // Create render texture for the UI
    RenderTexture2D buffer = LoadRenderTexture(screenWidth, screenHeight);
    GUI_State gui = GUI_MakeDefaultState(255);

    Game_State game_state = Game_MakeState();
    PLAYER_Actions player_actions = PLAYER_MakeActions();

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

        // UI
        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) gui.focus_id = 0;

        BeginTextureMode(buffer);
            ClearBackground(BLANK);
            GUI_TopBar(&gui, &player_actions);

            GUI_Window(1, "Window title", (Rectangle) { 20, 20, 250, 200 }, &gui);
        EndTextureMode();

        // Keyboard
        player_actions.toggle_character     |= IsKeyPressed(KEY_TAB);
        player_actions.move_down             = IsKeyDown(KEY_DOWN);
        player_actions.move_up               = IsKeyDown(KEY_UP);
        player_actions.move_left             = IsKeyDown(KEY_LEFT);
        player_actions.move_right            = IsKeyDown(KEY_RIGHT);

        // Actions
        if (player_actions.reset_characters) game_state = Game_MakeState();
        if (player_actions.add_character)    Game_AddCharacter(&game_state);  
        if (player_actions.toggle_character) Game_UpdateNextCharacter(&game_state);

        // Update character
        Game_Character *player = Game_GetCurrentCharacter(&game_state);
        if (player_actions.move_down)    player->Shape.y += CHARACTER_MAX_SPEED;
        if (player_actions.move_up)      player->Shape.y -= CHARACTER_MAX_SPEED;
        if (player_actions.move_left)    player->Shape.x -= CHARACTER_MAX_SPEED;
        if (player_actions.move_right)   player->Shape.x += CHARACTER_MAX_SPEED;

        // Update camera
        camera.target = (Vector2){ player->Shape.x, player->Shape.y };
        camera.zoom += ((float)GetMouseWheelMove() * 0.1f);
        if (camera.zoom > 3.0f) camera.zoom = 3.0f;
        else if (camera.zoom < 0.1f) camera.zoom = 0.1f;

        if (IsKeyPressed(KEY_KP_ADD) || IsKeyPressed(KEY_EQUAL)) gui.scale += 0.5;
        if (IsKeyPressed(KEY_KP_SUBTRACT) || IsKeyPressed(KEY_MINUS)) gui.scale -= 0.5;

        static float ui_opacity = 250.0;

        // 
        // RENDER
        //

        // Draw
        BeginDrawing();
            ClearBackground(RAYWHITE);

            // Game world
            BeginMode2D(camera);
                for (int i = 0; i < game_state.alive_characters; ++i ) {
                    Game_Character* c = &game_state.characters[i];
                    DrawRectangleRec(c->Shape, c->Color);
                }            
            EndMode2D();
            
            // Draw UI Buffer
            Rectangle sourceRec = { 0, 0, (float)buffer.texture.width, - (float)buffer.texture.height };
            DrawTextureRec(buffer.texture, sourceRec, (Vector2){ 0, 0 }, (Color){ 255, 255, 255, ui_opacity});
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
