#define UNITY_BUILD 1
#include <time.h> 
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
    int buttons = 3;
    float button_w = target.width / buttons;
    float button_h = target.height;

    GUI_BeginFontType(EGUI_FontType_GUI);
        actions->reset_characters    = GUI_Button("Reset",  (Rectangle) { button_w * 0, 0, button_w, button_h }, &icons->New, setup->theme.red);
        actions->add_character       = GUI_Button("Add",    (Rectangle) { button_w * 1, 0, button_w, button_h }, &icons->Open, setup->theme.gray);
        actions->toggle_character    = GUI_Button("Change", (Rectangle) { button_w * 2, 0, button_w, button_h }, &icons->Error, setup->theme.gray);
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

typedef struct {
    bool checkbox_value;
    char textbox_contents[256];
} Game_WindowState;

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


void WIN_window(GUI_Window* window, void* data)
{
    Game_WindowState *win_state = (Game_WindowState*)data;
    GUI_Setup *setup = GUI_GetSetup();
    EGUI_FontType font_type = EGUI_FontType_GUI;    
    float default_height = GUI_CalcDefaultHeightScaled(font_type);

    Rectangle window_workspace =
    GUI_BeginWindowContents(window, default_height * 2, true, font_type);
        GUI_BeginBlock(window_workspace.width / 3, default_height);
        // 1st textbox                
        GUI_Label("Hello", GUI_NextHorizontal(), window->colors);
        GUI_TextBox(2, win_state->textbox_contents, GUI_NextHorizontals(2), window->colors);
        
        // Switch
        GUI_BeginDuplicateBlock();
        GUI_Label("Switch", GUI_NextHorizontal(), window->colors);
        GUI_CheckBox(4, &win_state->checkbox_value, "ON", "OFF", GUI_NextHorizontals(2), setup->theme.red);
    GUI_EndWindowContents();
}

void WIN_layouts(GUI_Window* window, void* data)
{
    (void) data; // Supress unused warning.
    GUI_Setup* setup = GUI_GetSetup();
    EGUI_FontType font_type = EGUI_FontType_Default;
    float default_height = GUI_CalcDefaultHeightScaled(font_type);

    Rectangle window_workspace =
    GUI_BeginWindowContents(window, 0, false, EGUI_FontType_Default);

        // First block
        GUI_BeginBlock(window_workspace.width, default_height);
        GUI_Label("Some sample layouts for imKairos", GUI_NextVertical(), setup->theme.gray);

        // and more verticals of full width (can be written as Horizontals too, but requires
        // an explicit call to GUI_BeginBlock() to end each line)
        DrawDebugRect(GUI_Relative(GUI_NextVertical()), BROWN);                    
        DrawDebugRect(GUI_Relative(GUI_NextVertical()), BEIGE);

        // 1/3rd and 2/3rds blocks
        GUI_BeginBlock(window_workspace.width / 3, default_height);
        DrawDebugRect(GUI_Relative(GUI_NextHorizontal()), YELLOW);                    
        DrawDebugRect(GUI_Relative(GUI_NextHorizontals(2)), GREEN);

        // Second block
        // 3 horizontals of 1/3 of the available space
        GUI_BeginBlock(window_workspace.width / 3, default_height);
        DrawDebugRect(GUI_Relative(GUI_NextHorizontal()), DARKGRAY);
        DrawDebugRect(GUI_Relative(GUI_NextHorizontal()), GRAY);
        DrawDebugRect(GUI_Relative(GUI_NextHorizontal()), LIGHTGRAY);
        
        // Prepare for a new block with 5 elements per row
        GUI_BeginBlock(window_workspace.width / 5, -default_height);
        DrawDebugRect(GUI_Relative(GUI_NextHorizontal()), BLACK);
        DrawDebugRect(GUI_Relative(GUI_NextHorizontal()), BLACK);
        DrawDebugRect(GUI_Relative(GUI_NextHorizontal()), BLACK);
        DrawDebugRect(GUI_Relative(GUI_NextHorizontal()), BLACK);
        DrawDebugRect(GUI_Relative(GUI_NextHorizontal()), BLACK);
        
        GUI_BeginBlock(window_workspace.width / 2, default_height);
        DrawDebugRect(GUI_Relative(GUI_NextHorizontal()), RED);
        DrawDebugRect(GUI_Relative(GUI_NextHorizontal()), BLUE);
    GUI_EndWindowContents();
}

void WIN_winman(GUI_Window* window, void* data)
{
    (void) data; // Supress unused warning.

    GUI_State *state = GUI_GetState();
    EGUI_FontType font_type = EGUI_FontType_Default;
    float default_height = GUI_CalcDefaultHeightScaled(font_type);

    Rectangle window_workspace =
    GUI_BeginWindowContents(window, 0, false, font_type);
        GUI_BeginBlock(window_workspace.width, default_height);

        for (int i = 0; i < GUI_MAX_OPEN_WINS; ++i) {
            GUI_Window* win = &state->window_s[i];
            if (win->id == 0 || window->id == win->id) continue;
            if (GUI_Button(TextFormat("%d - %s", win->id, win->title), GUI_NextVertical(), NULL, window->colors)) {
                state->force_z_index = win->id;
            }
        }

        GUI_Window *active = GUI_GetWindowByZindex(0);
        Rectangle t_shape = GUI_WindowTitle(active->shape);
        Rectangle w_shape = active->shape;

        GUI_Label(TextFormat("scroll=%.2f   content_height=%.2f (>0 enables vertical scroll)", active->scroll_offset, active->content_height), GUI_NextVertical(), window->colors);

        GUI_Label("title_shape", GUI_NextVertical(), window->colors);
        GUI_Label(TextFormat("x=%.2f  y=%.2f", t_shape.x, t_shape.y), GUI_NextVertical(), window->colors);
        GUI_Label(TextFormat("w=%.2f  h=%.2f", t_shape.width, t_shape.height), GUI_NextVertical(), window->colors);

        GUI_Label("window_shape", GUI_NextVertical(), window->colors);
        GUI_Label(TextFormat("x=%.2f  y=%.2f", w_shape.x, w_shape.y), GUI_NextVertical(), window->colors);
        GUI_Label(TextFormat("w=%.2f  h=%.2f", w_shape.width, w_shape.height), GUI_NextVertical(), window->colors);

        Rectangle next = RelativeToRect(GUI_NextVertical(), window_workspace);
        GUI_DrawFace((Vector2){ next.x, next.y }, 16);
        next = RelativeToRect(GUI_NextVertical(), window_workspace);
        GUI_DrawFace((Vector2){ next.x, next.y }, 32);
        next = RelativeToRect(GUI_NextVertical(), window_workspace);
        GUI_DrawFace((Vector2){ next.x, next.y }, 64);
    GUI_EndWindowContents();
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
    Texture2D wallpaper         = GenerateVoronoiTexture((int)screen_max.x, (int)screen_max.y);
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
        if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
            pointer_style = pointer_style == EGUI_Pointer_Default ? EGUI_Pointer_AGS : EGUI_Pointer_Default;
        }

        GUI_BeginDraw(pointer_style);

        float topbar_height = GUI_CalcDefaultHeightScaled(EGUI_FontType_GUI);
        GUI_FontSetup *font_setup = GUI_GetFontSetup(EGUI_FontType_GUI);
        Rectangle window_limits = (Rectangle){ 
            0,
            topbar_height,
            GetScreenWidth(),
            GetScreenHeight() - topbar_height
        };

        BeginTextureMode(state.buffer);
            ClearBackground(BLANK);
            
            // Top bar
            GUI_TopBar(&player_actions, (Rectangle){ 0, 0, GetScreenWidth(), topbar_height });

            // Window(s)
            float win_third = window_limits.width / 3.0;
            static Game_WindowState win_state = {0};
            {
                static GUI_Window* win_window = NULL;
                if (win_window == NULL && !first_render)
                    win_window = GUI_MakeWindow(1, "Sample window", (Rectangle){ 20, 20, 250, 80 }, setup.theme.gray, NULL, WIN_window);
            }
            {
                static GUI_Window* win_layouts = NULL;
                if (win_layouts == NULL && !first_render) {
                    win_layouts = GUI_MakeWindow(2, "Sample layouts", (Rectangle){ 20, 20, 250, 200 }, setup.theme.gray, NULL, WIN_layouts);
                    win_layouts->shape.x         = win_third;
                    win_layouts->shape.y         = font_setup->default_height;
                }
                
                if (win_layouts != NULL) {                    
                    win_layouts->shape.width     = win_third;
                    win_layouts->shape.height    = window_limits.height;
                }
            }
            {
                static GUI_Window* win_man = NULL;
                if (win_man == NULL && !first_render)
                    win_man = GUI_MakeWindow(3, "WinMan", (Rectangle){ 20, 20, 250, 200 }, setup.theme.gray, &icons.Setup, WIN_winman);
                
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
                    GUI_Label(textbox_contents, RelativeToRect(GUI_NextVertical(), window_workspace), &gui, gui.theme.gray);
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

        if (IsKeyPressed(KEY_KP_ADD) || IsKeyPressed(KEY_EQUAL))      state.scale += 1.0;
        if (IsKeyPressed(KEY_KP_SUBTRACT) || IsKeyPressed(KEY_MINUS)) state.scale -= 1.0;

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
            ClearBackground(RAYWHITE);

            if (win_state.checkbox_value == 1) {
                DrawTextureRec(wallpaper, GetSourceRec(wallpaper), (Vector2){ 0, 0 }, setup.theme.gray.bg_color_3);
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