#define UNITY_BUILD 1
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "raylib.h"
#include "rlgl.h"
#include "raymath.h"
#include "env.h"
#include "str.h"
#include "rayext.h"

#include "gui_setup.h"
#include "gui_structs.h"
#include "gui.h"

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

typedef struct {
    bool reset_characters;
    bool add_character;
    bool toggle_character;
    bool move_up;
    bool move_down;
    bool move_left;
    bool move_right;
} PLAYER_Actions;

PLAYER_Actions PLAYER_MakeActions()
{
    PLAYER_Actions actions = { 0 };
    return actions;
}

void GUI_TopBar(PLAYER_Actions* actions, Rectangle target)
{
    GUI_Setup *setup = GUI_GetSetup();
    GUI_Icons *icons = GUI_GetIcons();
    GUI_Theme *theme = &setup->theme;
    int buttons = 3;
    float button_w = target.width / buttons;
    float button_h = target.height;

    Rectangle shape = (Rectangle) { target.x, target.y, button_w, button_h };
    GUI_BeginFontType(EGUI_FontType_GUI);
        actions->reset_characters    = GUI_Button(shape, "Reset", &icons->New, theme->red);
        actions->add_character       = GUI_Button(MoveRect(shape, (Vector2) { button_w, 0 }), "Add", &icons->Open, theme->gray);
        actions->toggle_character    = GUI_Button(MoveRect(shape, (Vector2) { button_w * 2, 0 }), "Change", &icons->Error, theme->gray);
    GUI_EndFontType();
}



#define CHARACTERS              4
#define CHARACTER_MAX_SPEED     6

typedef struct  {
    Rectangle Shape;
    Color Color;
} Game_Character;

typedef struct {
    int current_character;
    int alive_characters;
    Game_Character characters[CHARACTERS];
    Camera2D camera2D;
} Game_State;



Game_State Game_MakeState()
{
    Game_State state = {
        0,
        2,
        (Game_Character){ 0, 0, 10, 20, RED},
        (Game_Character){ 10, 30, 10, 20, BLUE},
        (Game_Character){ 50, 60, 10, 20, GREEN},
        (Game_Character){ 80, 60, 10, 20, ORANGE},
        { (Vector2){ 0, 0 }, { 0, 0 }, 0.0f, 1.0f }
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

Vector2 Game_GetCharacterCenter(Game_Character* character)
{
    return (Vector2){
        character->Shape.x + character->Shape.width / 2.0f,
        character->Shape.y + character->Shape.height / 2.0f
    };
}

bool Game_CheckRingCollision(Game_Character* character1, Game_Character* character2, float radius)
{
    Vector2 center1 = Game_GetCharacterCenter(character1);
    Vector2 center2 = Game_GetCharacterCenter(character2);
    float distance = Vector2Distance(center1, center2);
    return distance < (radius * 2);
}

void Game_UpdateCollisions(Game_State* state, bool collisions[], float radius)
{
    for (int i = 0; i < CHARACTERS; i++) {
        collisions[i] = false;
    }
    
    for (int i = 0; i < state->alive_characters; ++i) {
        for (int j = i + 1; j < state->alive_characters; ++j) {
            if (Game_CheckRingCollision(&state->characters[i], &state->characters[j], radius)) {
                collisions[i] = true;
                collisions[j] = true;
            }
        }
    }
}

//
// SAMPLE WINDOW
//

// 1. Define your data
typedef struct {
    bool checkbox_value;
    bool font_toggle;
    char input_contents[256];
    char input_int_contents[256];
    char input_float_contents[256];
} Game_WindowState;

// 2. Define your draw window
void WIN_window(GUI_Window* window, void* data)
{
    // Prepare your data
    Game_WindowState *win_state = (Game_WindowState*)data;

    // Responsive height (if you require it)
    // GUI_WindowUpdateShapeForContent(window);
    
    // Get the setup as it allows access to theming setup->theme.red
    GUI_Setup *setup            = GUI_GetSetup();
    // Keep or modify colors
    GUI_ThemeColors colors      = window->colors;
    // Set your font
    EGUI_FontType font_type     = win_state->font_toggle ? EGUI_FontType_GUI: EGUI_FontType_Default;    
    // And define your UI
    Rectangle window_workspace  =
    GUI_BeginWindowContents(window, font_type);
        // A default layout with 3 columns
        GUI_BeginBlockCols(3, window_workspace, font_type);

        // 1st input (textbox)
        GUI_Text(GUI_NextHorizontal(), "Text", colors);
        GUI_Input(GUI_NextHorizontals(2), win_state->input_contents, EGUI_InputText, colors);

        // 2nd input for integer
        // TODO@dc: add min, max and parsing
        GUI_BeginDuplicateBlock();
        GUI_Text(GUI_NextHorizontal(), "Int", colors);
        GUI_Input(GUI_NextHorizontals(2), win_state->input_int_contents, EGUI_InputInt, colors);

        // 3rd input for float
        GUI_BeginDuplicateBlock();
        GUI_Text(GUI_NextHorizontal(), "Float", colors);
        GUI_Input(GUI_NextHorizontals(2), win_state->input_float_contents, EGUI_InputFloat, colors);

        // Wallpaper check (checkbox/switch)
        // With a theme.red color
        GUI_BeginDuplicateBlock();
        GUI_Text(GUI_NextHorizontal(), "Wallpaper",  colors);
        GUI_Check(GUI_NextHorizontals(2), &win_state->checkbox_value, "ON", "OFF", setup->theme.red);

        // Font toggler
        GUI_BeginDuplicateBlock();
        GUI_Text(GUI_NextHorizontal(), "Font",  colors);
        GUI_Check(GUI_NextHorizontals(2), &win_state->font_toggle, "GUI", "DEF", colors);
    GUI_EndWindowContents(window);
}

void WIN_layouts(GUI_Window* window, void* data)
{
    (void) data; // Supress unused warning.
    GUI_Setup* setup = GUI_GetSetup();
    EGUI_FontType font_type = EGUI_FontType_Default;
    float default_height = GUI_CalcDefaultHeightScaled(font_type);

    Rectangle window_workspace =
    GUI_BeginWindowContents(window, EGUI_FontType_Default);

        // First block
        GUI_BeginBlock(window_workspace.width, default_height);
        GUI_Text(GUI_NextVertical(), "Some sample layouts for imKairos", setup->theme.gray);

        // and more verticals of full width (can be written as Horizontals too, but requires
        // an explicit call to GUI_BeginBlock() to end each line)
        GUI_BeginControlScissor();
            float color_alpha = 0.9;
            DrawDebugRect(GUI_Relative(GUI_NextVertical()), ColorAlpha(BROWN, color_alpha));
            DrawDebugRect(GUI_Relative(GUI_NextVertical()), ColorAlpha(BEIGE, color_alpha));

            // 1/3rd and 2/3rds blocks
            GUI_BeginBlock(window_workspace.width / 3, default_height);
            DrawDebugRect(GUI_Relative(GUI_NextHorizontal()), ColorAlpha(YELLOW, color_alpha));                    
            DrawDebugRect(GUI_Relative(GUI_NextHorizontals(2)), ColorAlpha(GREEN, color_alpha));

            // Second block
            // 3 horizontals of 1/3 of the available space
            GUI_BeginBlock(window_workspace.width / 3, default_height);
            DrawDebugRect(GUI_Relative(GUI_NextHorizontal()), ColorAlpha(DARKGRAY, color_alpha));
            DrawDebugRect(GUI_Relative(GUI_NextHorizontal()), ColorAlpha(GRAY, color_alpha));
            DrawDebugRect(GUI_Relative(GUI_NextHorizontal()), ColorAlpha(LIGHTGRAY, color_alpha));
            
            // Prepare for a new block with 5 elements per row
            // You can send negative values to use AVAILABLE - YOUR_VALUE
            // Ex:
            // -default_height means take all space minus a default_height to insert a final row
            GUI_BeginBlock(window_workspace.width / 5, -default_height);
            DrawDebugRect(GUI_Relative(GUI_NextHorizontal()), ColorAlpha(BLACK, 0.1));
            DrawDebugRect(GUI_Relative(GUI_NextHorizontal()), ColorAlpha(BLACK, 0.2));
            DrawDebugRect(GUI_Relative(GUI_NextHorizontal()), ColorAlpha(BLACK, 0.3));
            DrawDebugRect(GUI_Relative(GUI_NextHorizontal()), ColorAlpha(BLACK, 0.4));
            DrawDebugRect(GUI_Relative(GUI_NextHorizontal()), ColorAlpha(BLACK, 0.5));
            
            // Final row
            GUI_BeginBlock(window_workspace.width / 2, default_height);
            DrawDebugRect(GUI_Relative(GUI_NextHorizontal()), ColorAlpha(RED, color_alpha));
            DrawDebugRect(GUI_Relative(GUI_NextHorizontal()), ColorAlpha(BLUE, color_alpha));
        EndScissorMode();
    GUI_EndWindowContents(window);
}

void WIN_winman(GUI_Window* window, void* data)
{
    (void) data; // Supress unused warning.

    GUI_State *state = GUI_GetState();
    GUI_Setup *setup = GUI_GetSetup();
    GUI_Icons *icons = GUI_GetIcons();
    EGUI_FontType font_type = EGUI_FontType_Default;
    float default_height = GUI_CalcDefaultHeightScaled(font_type);

    Rectangle window_workspace =
    GUI_BeginWindowContents(window, font_type);
        GUI_BeginBlock(window_workspace.width, default_height);

        static GUI_Window* win_window = NULL;
        if (GUI_Button(GUI_NextVertical(), "Open sample window", NULL, window->colors)) {
            if (win_window == NULL || win_window->id == 0) {
                win_window = GUI_MakeWindow(2, "Sample window", (Rectangle){ 20, 20, 300, 100 }, setup->theme.gray, &icons->Dog, false, WIN_window);
            }
        }
        static GUI_Window* win_layouts = NULL;
        if (GUI_Button(GUI_NextVertical(), "Open layouts window", NULL, window->colors)) {
            if (win_layouts == NULL || win_layouts->id == 0) {
                win_layouts = GUI_MakeWindow(3, "Layouts window", (Rectangle){ 20, 20, 300, 100 }, setup->theme.gray, &icons->Layouts, false, WIN_layouts);
            }
        }

        GUI_Text(GUI_NextVertical(), "--- Opened windows ---", window->colors);
        GUI_BeginDuplicateBlock();
        for (int i = 0; i < GUI_MAX_OPEN_WINS; ++i) {
            GUI_Window* win = &state->window_s[i];
            if (win->id == 0 || window->id == win->id) continue;
            if (GUI_Button(GUI_NextVertical(), TextFormat("%d - %s", win->id, win->title), NULL, window->colors)) {
                state->force_z_index = win->id;
            }
        }
        GUI_Text(GUI_NextVertical(), "--- End opened windows ---", window->colors);

        GUI_Window *active = GUI_GetWindowByZindex(0);
        if (active != NULL) {
            Rectangle t_shape = GUI_WindowTitle(active->shape);
            Rectangle w_shape = active->shape;

            GUI_Text(GUI_NextVertical(), "--- Focused window ---", window->colors);
            GUI_Text(GUI_NextVertical(), TextFormat("ID=%d scroll=%.2f content_height=%.2f", active->id, active->scroll_offset, active->content_height),  window->colors);

            GUI_Text(GUI_NextVertical(), "title_shape", window->colors);
            GUI_Text(GUI_NextVertical(), TextFormat("x=%.2f  y=%.2f w=%.2f  h=%.2f", t_shape.y,  t_shape.x, t_shape.width, t_shape.height), window->colors);

            GUI_Text(GUI_NextVertical(), "window_shape", window->colors);
            GUI_Text(GUI_NextVertical(), TextFormat("x=%.2f  y=%.2f w=%.2f  h=%.2f", w_shape.x, w_shape.y, w_shape.width, w_shape.height),  window->colors);
            GUI_Text(GUI_NextVertical(), "--- End focused window ---", window->colors);
        }

        Rectangle next = GUI_NextVertical();
        float icon_w = GUI_GetIconWidth();
        GUI_Face((Vector2){ next.x, next.y }, (float) icon_w / 2);
        GUI_Face((Vector2){ next.x + (float) icon_w / 2, next.y }, icon_w);
        GUI_Face((Vector2){ next.x + (float) icon_w / 2 + icon_w, next.y }, icon_w * 2);
        GUI_Icon(&icons->Dog, (Vector2){ next.x + (float) icon_w / 2 + icon_w * 3, next.y }, icon_w * 2, WHITE);
        GUI_NextVertical(); // Jump line
        GUI_NextVertical(); // Jump line

        static Texture2D image;
        if (image.id == 0) image = LoadTexture("art/abstractica.png");
        next = GUI_NextVertical();
        GUI_Image(image, (Rectangle){ next.x, next.y, next.width, 320 });
    GUI_EndWindowContents(window);
}

const char* BuildTimeFormatted()
{
    static char buffer[32];

    // Parse __TIME__ HH:MM:SS
    int h = (__TIME__[0]-'0')*10 + (__TIME__[1]-'0');
    int m = (__TIME__[3]-'0')*10 + (__TIME__[4]-'0');
    int s = (__TIME__[6]-'0')*10 + (__TIME__[7]-'0');

    const char* ampm = "am";
    if(h >= 12) ampm = "pm";
    if(h > 12)  h -= 12; 
    if(h == 0)  h = 12; // midnight edge case

    snprintf(buffer, sizeof(buffer), "%02dh:%02dm:%02ds %s", h, m, s, ampm);
    return buffer;
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


int main(void) {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);

    InitWindow(DEV_WINDOW_W, DEV_WINDOW_H, TextFormat("%s - %s - %s", BuildTimeFormatted(), __DATE__, GetWorkingDirectory()));
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

    // PREPARE GUI
    // Create render texture for the GUI
    GUI_State state             = GUI_MakeStateDefault(screen_max);
    GUI_Setup setup             = GUI_MakeSetupDefault();
    GUI_Icons icons             = setup.icon_setup.icons;
    Texture2D wp_voronoi        = GenerateVoronoiTexture((int)screen_max.x, (int)screen_max.y);
    
    GUI_SetContext(&state, &setup);

    // PREPARE GAME
    Game_State game_state = Game_MakeState();
    PLAYER_Actions player_actions = PLAYER_MakeActions();

    SetTargetFPS(60);
    bool first_render = true;
    while (!WindowShouldClose()) {
        //
        // UPDATE
        //

        // UI
        static EGUI_Pointer pointer_style = EGUI_Pointer_Default;
        if (IsKeyPressed(KEY_F10)) {
            pointer_style = pointer_style == EGUI_Pointer_Default ? EGUI_Pointer_AGS : EGUI_Pointer_Default;
        }

        GUI_BeginDraw(pointer_style);

        float topbar_height = GUI_CalcDefaultHeightScaled(EGUI_FontType_GUI);
        Rectangle window_limits = (Rectangle){ 
            0,
            topbar_height,
            GetScreenWidth(),
            GetScreenHeight() - topbar_height
        };
        GUI_SetWindowLimits(window_limits);

        BeginTextureMode(state.buffer);
            ClearBackground(BLANK);
            
            // Top bar
            GUI_TopBar(&player_actions, (Rectangle){ 0, 0, GetScreenWidth(), topbar_height });

            // Window(s)
            float win_third = window_limits.width / 3.0;
            static Game_WindowState win_state = {0};
            {
                static GUI_Window* win_man = NULL;
                if (win_man == NULL && !first_render)
                    win_man = GUI_MakeWindow(1, "WinMan", (Rectangle){ 20, 20, 250, 200 }, setup.theme.gray, &icons.Setup, true, WIN_winman);
                
                if (win_man != NULL) {
                    win_man->shape.x         = win_third * 2;
                    win_man->shape.y         = window_limits.y;
                    win_man->shape.width     = win_third;
                    win_man->shape.height    = window_limits.height;
                }
            }

            GUI_UpdateAndDrawWindows(window_limits, &win_state);
            
        #if 0
            static Rectangle win_debug = { 20, 220, 350, 200 };
            float win_third     = window_limits.width / 3.0;
            win_debug.x         = win_third * 2;
            win_debug.y         = window_limits.y;
            win_debug.width     = win_third;
            win_debug.height    = window_limits.height;
            
            void (*win_debug_contents)
            GUI_Window(4, "Kairos Debug", &gui, &win_debug, window_limits, gui.theme.gray);
            {
                // Window contents
                Rectangle window_workspace =
                GUI_BeginWindowContents(win_debug, &gui);
                    GUI_BeginVertical(gui.default_height);
                    GUI_BeginHorizontal(window_workspace.width);
                    GUI_Text(textbox_contents, RelativeToRect(GUI_NextVertical(), window_workspace), &gui, gui.theme.gray);
                GUI_EndWindowContents();
            }

            static Rectangle win_layouts = { 20, 220, 350, 200 };
            win_layouts.x         = win_third;
            win_layouts.y         = window_limits.y;
            win_layouts.width     = win_third;
            win_layouts.height    = window_limits.height;
            GUI_Window(500, "Kairos Layouts", &gui, &win_layouts, window_limits, gui.theme.gray);
            {
                // Window contents
                Rectangle window_workspace =
                
            }
        #endif
            GUI_DrawPointer();
        EndTextureMode();

        GUI_EndDraw();
        
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
        Camera2D *camera = &game_state.camera2D;
        camera->target = (Vector2){ player->Shape.x, player->Shape.y };
        camera->offset = (Vector2){ window_limits.width / 2.0f,  window_limits.height / 2.0f };
        camera->zoom += ((float)GetMouseWheelMove() * 0.1f);
        if (camera->zoom > 3.0f) camera->zoom = 3.0f;
        else if (camera->zoom < 0.1f) camera->zoom = 0.1f;

        if (IsKeyPressed(KEY_F12)) state.scale += 1.0;
        if (IsKeyPressed(KEY_F11)) state.scale -= 1.0;

        static float ui_opacity = 255.0;

        // 
        // RENDER
        //

        // FXs
        static RenderTexture2D rain_buffer  = { 0 };     
        if (rain_buffer.id == 0) {
            rain_buffer = LoadRenderTexture((int)(GetScreenWidth() / 5), (int)(GetScreenHeight() / 5 + 100));
        }
        BeginTextureMode(rain_buffer);
            ClearBackground(BLANK);
            DrawRain(rain_buffer.texture.width, rain_buffer.texture.height, 2.5, ColorAlpha(BLUE, 0.4f));
        EndTextureMode();

        // Draw
        BeginDrawing();
            ClearBackground(WHITE);

            if (win_state.checkbox_value == 1) {
                DrawTextureRec(wp_voronoi, GetSourceRec(wp_voronoi), (Vector2){ 0, 0 }, setup.theme.gray.bg_color_3);
                DrawTexturePro(rain_buffer.texture, GetSourceRec(rain_buffer.texture), MoveAndExtendXY(window_limits, 0, 100), (Vector2){0,0}, 0.0, WHITE);
            }

            // Game world
            BeginMode2D(*camera);
                bool collisions[CHARACTERS];
                float radius = 30.0f;
                Game_UpdateCollisions(&game_state, collisions, radius);
                
                for (int i = 0; i < game_state.alive_characters; ++i) {
                    Game_Character* c = &game_state.characters[i];
                    
                    Vector2 center = Game_GetCharacterCenter(c);
                    
                    Color ring_color = collisions[i] ? RED : BLACK;
                    DrawRing(center, radius-3, radius, 0, 360, 32, ring_color);
                    
                    DrawRectangleRec(c->Shape, c->Color);
                }
            EndMode2D();
            
            // Draw UI Buffer
            {
                DrawTextureRec(state.buffer.texture, FlipYRec(GetSourceRec(state.buffer.texture)), (Vector2){ 0, 0 }, (Color){ 255, 255, 255, ui_opacity});
            }
        EndDrawing();

        first_render = false;
    }

    CloseWindow();
    return 0;
}