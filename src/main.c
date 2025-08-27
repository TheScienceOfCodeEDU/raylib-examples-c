#include <string.h>
#define UNITY_BUILD 1
#include "raylib.h"
#include "raymath.h"
#include "rayext.h"
#include "env.h"


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
    Color bg_color_0;       // Primary background color
    Color bg_color_1;       // Secondary background color
    Color bg_color_2;       // Tertiary background color
    Color color_0;          // Primary color (e.g., for text or elements)
    Color color_1;          // Secondary color
    Color color_2;          // Tertiary color
    Color b_color_0;        // Primary border color
    Color b_color_1;        // Secondary border color
    Vector2 padding;        // Internal padding
    float border;           // Border thickness
    bool font_custom;       // Indicates if a custom font is used
    float font_spacing;     // Font spacing
    Vector2 blink_size;     // Size of the blinking cursor
} typedef GUI_Theme;


GUI_Theme GUI_MakeDefaultTheme(int opacity)
{
    GUI_Theme theme = {
        // Background colors
        (Color) { 80, 67, 48, opacity },    // bg_color_0: Dark brown with variable opacity
        (Color) { 116, 100, 67, opacity },  // bg_color_1: Medium brown with variable opacity
        (Color) { 58, 49, 35, opacity },    // bg_color_2: Very dark brown with variable opacity

        // Primary colors
        (Color) { 171, 158, 127, 255 },    // color_0: Light beige, fully opaque
        (Color) { 238, 208, 147, 255 },    // color_1: Warm beige, fully opaque
        (Color) { 253, 250, 85, 255 },     // color_2: Light yellow, fully opaque

        // Border colors
        (Color) { 33, 33, 33, 200 },       // b_color_0: Very dark gray, semi-opaque
        (Color) { 118, 118, 118, 200 },    // b_color_1: Medium gray, semi-opaque

        // Padding
        (Vector2) { 8, 8 },                // padding
        2.0f,                              // border
        1,                                 // font_custom
        1.0,                               // font_spacing
        (Vector2) { 2, 10 }                // blink_size
    };

    return theme;
}

struct {
    GUI_Theme theme;
    GUI_Icons icons;
    float scale;
    Font font_custom;

    int window_focus_id;
    bool window_focus_moving;
    int control_focus_id;
    Vector2 mouse_last;
    Vector2 mouse_current;

    float default_height;
} typedef GUI_State;

GUI_State GUI_MakeDefaultState(float opacity)
{
    GUI_State state = {
        GUI_MakeDefaultTheme(255),
        GUI_LoadIcons(),
        2.0f,
        LoadFont("fnt/pixelplay.png"),

        -1,
        0,
        0,
        (Vector2){ 0.0, 0.0},
        (Vector2){ 0.0, 0.0},

        0
    };
    //SetTextureFilter(state.font.texture, TEXTURE_FILTER_POINT);
    return state;
}

Font GUI_GetFont(GUI_Theme theme, Font font_custom)
{
    Font font = theme.font_custom ? font_custom : GetFontDefault();
    return font;
}

float GUI_CalcDefaultHeight(GUI_State* gui)
{
    Font font = GUI_GetFont(gui->theme, gui->font_custom);
    Vector2 textShape = MeasureTextEx(GetFontDefault(), "Hello raylib", font.baseSize, gui->theme.font_spacing);
    return textShape.y;
}

float GUI_CalcShapeAvailableHeight(Rectangle shape, GUI_State* gui)
{
    return (shape.height - gui->theme.padding.y * 2) * gui->scale;
}

float GUI_CalcDefaultScaledHeight(GUI_State* gui)
{
    return (GUI_CalcDefaultHeight(gui) + gui->theme.border) * gui->scale + gui->theme.padding.y * 2;
}

float GUI_CalcDefaultIconSize(GUI_State* gui)
{
    return (GUI_CalcDefaultHeight(gui) + gui->theme.border) * gui->scale;
}


void GUI_DrawBorders(Rectangle shape, Color dark, Color light, float border)
{
    // Draw top border (horizontal line)    
    DrawRectangle(shape.x, shape.y, shape.width, border, dark);

    // Draw left border (vertical line)
    DrawRectangle(shape.x, shape.y, border, shape.height, dark);

    // Draw bottom border (horizontal line)
    DrawRectangle(shape.x, shape.y + shape.height - border, shape.width, border, light);

    // Draw right border (vertical line)
    DrawRectangle(shape.x + shape.width - border, shape.y, border, shape.height, light);

}

void GUI_DrawButton(char* text, Rectangle shape,  GUI_ElementStatus status, GUI_Theme theme, Font font_custom, float scale, float icon_w) 
{
    Color bg_color = status == GUI_Status_Default ? theme.bg_color_0 : theme.bg_color_1;
    Color tx_color = status == GUI_Status_Default ? theme.color_0 : theme.color_1;

    Color b_light = status == GUI_Status_Click ? theme.b_color_1 : theme.b_color_0;
    Color b_dark = status == GUI_Status_Click ? theme.b_color_0 : theme.b_color_1;

    DrawRectangleRec(shape, bg_color);
    GUI_DrawBorders(shape, b_dark, b_light, theme.border * scale);

    Font font = GUI_GetFont(theme, font_custom);
    DrawTextEx(font, text, 
        (Vector2){ shape.x + icon_w + theme.padding.x * 2, shape.y + theme.padding.y}, 
        font.baseSize * scale, theme.font_spacing, tx_color);
}

void GUI_Icon(Texture2D* texture2d, Vector2 position, float height, float scale, Color tint)
{
    scale *= height / texture2d->height;
    DrawTextureEx(*texture2d, position, 0, scale, tint);
}

bool GUI_Button(char* text, Rectangle shape, GUI_State* gui, Texture2D* icon)
{
    GUI_Theme theme = gui->theme;
    Vector2 mouse = GetMousePosition();
    GUI_ElementStatus status = GUI_Status_Default;

    bool collide            = CheckCollisionPointRec(mouse, shape);
    bool moving_window      = gui->window_focus_moving == 0;
    bool focusable          = collide && moving_window;
    if (focusable) {
        if (!IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            status = GUI_Status_Focus;
        } else {
            status = GUI_Status_Click;
        }
    }
    
    float icon_w = GUI_CalcDefaultIconSize(gui);
    GUI_DrawButton(text, shape, status, theme, gui->font_custom, gui->scale, icon_w);
    GUI_Icon(icon, 
        (Vector2) { shape.x + theme.border * gui->scale, shape.y + theme.border * gui->scale }, 
        icon_w, 1.0f, WHITE);

    return collide && IsMouseButtonReleased(MOUSE_BUTTON_LEFT);
}

void GUI_DrawTextBox(char* value, Rectangle shape,  GUI_ElementStatus status, GUI_Theme theme, Font font_custom, float scale)
{
    const float blink_speed     = 0.5f;    
    static float blink_timer    = 0.0f;
    static bool blink_state     = 0;    

    // Update cursor blink timer
    if (blink_state)    blink_timer -= GetFrameTime();
    else                blink_timer += GetFrameTime();

    Color bg_color = status == GUI_Status_Default ? theme.bg_color_0 : theme.bg_color_1;
    Color tx_color = status == GUI_Status_Default ? theme.color_0 : theme.color_1;

    Color b_light = status == GUI_Status_Focus ? theme.b_color_1 : theme.b_color_0;
    Color b_dark = status == GUI_Status_Focus ? theme.b_color_0 : theme.b_color_1;

    DrawRectangleRec(shape, bg_color);
    GUI_DrawBorders(shape, b_dark, b_light, theme.border * scale);

    Font font = GUI_GetFont(theme, font_custom);
    DrawTextEx(font, value, 
        (Vector2){ shape.x + theme.padding.x, shape.y + theme.padding.y}, 
        font.baseSize * scale, theme.font_spacing, tx_color);

    if (status == GUI_Status_Focus && blink_state) {
        int text_w = MeasureTextEx(font, value, font.baseSize * scale, theme.font_spacing).x;
        int text_h = MeasureTextEx(font, value, font.baseSize * scale, theme.font_spacing).y;
        DrawRectangle(shape.x + theme.padding.x + text_w, shape.y + theme.padding.y, theme.blink_size.x * scale, text_h, tx_color);
    }
    
    if (blink_timer > blink_speed)  blink_state = 1;
    if (blink_timer < 0)            blink_state = 0;
}

bool GUI_TextBox(int id, char* value, Rectangle shape, GUI_State* gui)
{
    GUI_Theme theme = gui->theme;
    Vector2 mouse = GetMousePosition();
    GUI_ElementStatus status = GUI_Status_Default;

    bool collide        = CheckCollisionPointRec(gui->mouse_current, shape);
    bool interacting    = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);

    bool focusable      = collide && interacting;
    if (focusable) {
        gui->control_focus_id = id;
    }
    
    status = gui->control_focus_id == id ? GUI_Status_Focus : GUI_Status_Default;

    if (status == GUI_Status_Focus) {
        int textLength = strlen(value);
        // Handle text input
        int key = GetCharPressed();
        while (key > 0) {
            if (key >= 32 && key <= 126 && textLength < 255) { // Printable ASCII characters
                value[textLength] = (char)key;
                textLength++;
                value[textLength] = '\0'; // Null-terminate string
            }
            key = GetCharPressed(); // Get next queued character
        }

        // Handle backspace
        if (IsKeyPressed(KEY_BACKSPACE) && textLength > 0) {
            textLength--;
            value[textLength] = '\0';
        }
    }

    GUI_DrawTextBox(value, shape, status, gui->theme, gui->font_custom, gui->scale);
        
    return false;
}

void GUI_DrawWindow(char* title, Rectangle shape, Rectangle shapeTitle,  GUI_ElementStatus status, GUI_Theme theme, Font font_custom, float scale, bool icon)
{
    Color bg_color = status == GUI_Status_Default ? theme.bg_color_2 : theme.bg_color_1;
    Color tx_color = theme.color_0;

    Color b_light = status == GUI_Status_Click ? theme.b_color_1 : theme.b_color_0;
    Color b_dark = status == GUI_Status_Click ? theme.b_color_0 : theme.b_color_1;
    
    DrawRectangleRec(shape, bg_color);

    GUI_DrawBorders(shape, b_dark, b_light, theme.border * scale);

    Font font = GUI_GetFont(theme, font_custom);
    DrawTextEx(font, title,
        (Vector2){shape.x + theme.padding.x, shape.y + theme.padding.y}, 
        font.baseSize * scale, theme.font_spacing, tx_color);

    GUI_DrawBorders(shapeTitle, b_light, b_dark, theme.border * scale);
}

bool GUI_CheckCollisionPointRecWithMargin(Vector2 point, Rectangle rect, float margin) {
    Rectangle inner = {
        rect.x + margin,
        rect.y + margin, 
        rect.width - 2 * margin,
        rect.height - 2 * margin
    };

    return CheckCollisionPointRec(point, inner);
}

Rectangle GUI_WindowTitle(Rectangle shape, GUI_State* gui)
{
    Rectangle shapeTitle = {
        shape.x,
        shape.y,
        shape.width,
        gui->default_height
    };
    return shapeTitle;
}

Rectangle GUI_WindowWorkspace(Rectangle shape, GUI_State* gui)
{
    Rectangle shape_title = GUI_WindowTitle(shape, gui);
    Rectangle shape_workspace = {
        shape.x,
        shape.y + shape_title.height,
        shape.width,
        shape.height - shape_title.height
    };

    if (DEV_DEBUG_GUI) {
        DrawRectangleRec(shape_title, ColorAlpha(ORANGE, 0.5));
        DrawRectangleRec(shape_workspace, ColorAlpha(GREEN, 0.5));
    }
    return shape_workspace;
}

void GUI_Window(int id, char* title, GUI_State* gui, Rectangle *shape,  Rectangle limits)
{
    Rectangle shape_title = GUI_WindowTitle(*shape, gui);
    
    // Conditions
    bool collide        = CheckCollisionPointRec(gui->mouse_current, *shape);
    bool collide_title  = CheckCollisionPointRec(gui->mouse_current, shape_title);
    bool interacting    = IsMouseButtonDown(MOUSE_BUTTON_LEFT);
    
    // Focus
    bool focusing = collide && gui->window_focus_id == 0;
    if (focusing) {
        gui->window_focus_id        = id;
        gui->window_focus_moving    = collide_title;
    }

    // Movement
    bool moving = interacting && gui->window_focus_moving;
    if (moving) {
        Vector2 displacement    = Vector2Subtract(gui->mouse_current, gui->mouse_last);
        shape->x                += displacement.x;
        shape->y                += displacement.y;
        shape_title.x           += displacement.x;
        shape_title.y           += displacement.y;
    } else {
        gui->window_focus_moving = false;
    }

    // Limit
    *shape          = LimitRect(*shape, limits);
    shape_title     = LimitRect(shape_title, limits);

    // Draw
    GUI_ElementStatus status = gui->window_focus_id == id ? GUI_Status_Focus : GUI_Status_Default;
    GUI_DrawWindow(title, *shape, shape_title, status, gui->theme, gui->font_custom, gui->scale, false);
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

void GUI_TopBar(GUI_State* gui, PLAYER_Actions* actions, Rectangle target)
{
    int buttons = 3;
    float screen_w = target.width;
    float button_w = target.width / buttons;
    float button_h = target.height;

    actions->reset_characters    = GUI_Button("Reset", (Rectangle) { button_w * 0, 0, button_w, button_h }, gui, &gui->icons.New);
    actions->add_character       = GUI_Button("Add", (Rectangle) { button_w * 1, 0, button_w, button_h }, gui, &gui->icons.Open);
    actions->toggle_character    = GUI_Button("Change", (Rectangle) { button_w * 2, 0, button_w, button_h }, gui, &gui->icons.Save);
}

Rectangle GUI_RelativeToRect(Rectangle rectangle, Rectangle relativeTo)
{
    Rectangle result = {
        rectangle.x + relativeTo.x,
        rectangle.y + relativeTo.y,
        rectangle.width > relativeTo.width ? relativeTo.width : rectangle.width,
        rectangle.height > relativeTo.height ? relativeTo.height : rectangle.height
    };

    return result;
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
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);

    InitWindow(DEV_WINDOW_W, DEV_WINDOW_H, TextFormat("Raylib Movement - %s", GetWorkingDirectory()));
    SetTargetFPS(60);

    while (GetCurrentMonitor() != DEV_TARGET_MONITOR && DEV_TARGET_MONITOR < GetMonitorCount())
        SetWindowMonitor(DEV_TARGET_MONITOR);

    Vector2 screen_max = (Vector2) { GetMonitorWidth(DEV_TARGET_MONITOR),  GetMonitorHeight(DEV_TARGET_MONITOR) };
    SetWindowMaxSize(screen_max.x, screen_max.y);

    if (DEV_FULLSCREEN)
        ToggleFullscreen();

    if (DEV_MAXIMIZE)
        MaximizeWindow();

    if (DEV_HIDE_CURSOR)
        HideCursor();

    // Create render texture for the UI
    RenderTexture2D buffer = LoadRenderTexture(screen_max.x, screen_max.y);
    GUI_State gui = GUI_MakeDefaultState(255);
    Texture2D mouse_texture = LoadTexture("ico/cursor.png");

    Game_State game_state = Game_MakeState();
    PLAYER_Actions player_actions = PLAYER_MakeActions();

    Camera2D camera = { 0 };
    camera.target = (Vector2){ 0, 0 };
    camera.offset = (Vector2){ screen_max.x / 2.0f, screen_max.y / 2.0f };
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;

    SetTargetFPS(60);

    Rectangle window = (Rectangle) { 20, 20, 250, 200 };
    char textbox_contents[256] = "hello\0";
    
    while (!WindowShouldClose()) {
        //
        // UPDATE
        //

        // UI
        gui.default_height = GUI_CalcDefaultScaledHeight(&gui);
        Rectangle mouse_limits = (Rectangle) {
            0,
            0,
            GetScreenWidth(),
            GetScreenHeight()
        };

        gui.mouse_current = LimitVector2(GetMousePosition(), mouse_limits);

        Vector2 mouse_shape = (Vector2){
            gui.mouse_current.x - (mouse_texture.width * gui.scale * 0.5f),
            gui.mouse_current.y - (mouse_texture.height * gui.scale * 0.5f),
        };

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            gui.control_focus_id = 0;
        }

        if (gui.window_focus_id == 0){
            if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
                gui.window_focus_id = -1;
                gui.window_focus_moving = 0;
            }
        } else {
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                gui.window_focus_id = 0;
                gui.window_focus_moving = 0;
            }
        }

        Rectangle window_limits = (Rectangle){ 
            0,
            gui.default_height,
            GetScreenWidth(),
            GetScreenHeight() 
        };

        BeginTextureMode(buffer);
            ClearBackground(BLANK);
            
            // Top bar
            GUI_TopBar(&gui, &player_actions, (Rectangle){ 0, 0, GetScreenWidth(), gui.default_height });

            // Window
            GUI_Window(1, "Window title", &gui, &window, window_limits);
            {
                Rectangle window_workspace = GUI_WindowWorkspace(window, &gui);

                // Window contents
                Rectangle textbox = (Rectangle){ 0, 0, GetScreenWidth(), gui.default_height };
                GUI_TextBox(1, textbox_contents, GUI_RelativeToRect(textbox, window_workspace), &gui);
            }
            
            DrawTextureEx(mouse_texture, mouse_shape, 0, gui.scale, WHITE);            
        EndTextureMode();

        gui.mouse_last = gui.mouse_current;
        
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

        if (IsKeyPressed(KEY_KP_ADD) || IsKeyPressed(KEY_EQUAL)) gui.scale += 1.0;
        if (IsKeyPressed(KEY_KP_SUBTRACT) || IsKeyPressed(KEY_MINUS)) gui.scale -= 1.0;

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
