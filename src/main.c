#define UNITY_BUILD 1
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "raylib.h"
#include "rlgl.h"
#include "raymath.h"
#include "env.h"
#include "rayext.h"

#include "gui_setup.h"
#include "gui_structs.h"
#include "gui.h"

#include "game_structs.h"
#include "game_gui.h"
#include "game.h"
#include "experiments.h"

#define GAME_RES_W          320
#define GAME_RES_H          240
#define GAME_RES_HALF_W     160
#define GAME_RES_HALF_H     120



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
    Game_State *game_state;
} Game_WindowState;

Game_WindowState Game_MakeWindowState(Game_State *game_state)
{
    Game_WindowState state      = {
        .checkbox_value         = false,
        .font_toggle            = false,
        .input_contents         = {'\0'},
        .input_int_contents     = {'\0'},
        .input_float_contents   = {'\0'},
        .game_state             = game_state
    };
    return state;
}


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



void WIN_character_debug(GUI_Window* window, void* data)
{
    Game_State *game_state      =((Game_WindowState*)data)->game_state;
    Game_Character *ch          = &game_state->characters[game_state->current_character];
    GUI_ThemeColors colors      = window->colors;
    EGUI_FontType font_type     = EGUI_FontType_Default;

    Rectangle workspace = GUI_BeginWindowContents(window, font_type);
        GUI_BeginBlockCols(2, workspace, font_type);

        // Shape
        GUI_Text(GUI_NextHorizontal(), "shape.x", colors);
        GUI_Text(GUI_NextHorizontal(), TextFormat("%.2f", ch->shape.x), colors);

        GUI_BeginDuplicateBlock();
        GUI_Text(GUI_NextHorizontal(), "shape.y", colors);
        GUI_Text(GUI_NextHorizontal(), TextFormat("%.2f", ch->shape.y), colors);

        GUI_BeginDuplicateBlock();
        GUI_Text(GUI_NextHorizontal(), "shape.w", colors);
        GUI_Text(GUI_NextHorizontal(), TextFormat("%.2f", ch->shape.width), colors);

        GUI_BeginDuplicateBlock();
        GUI_Text(GUI_NextHorizontal(), "shape.h", colors);
        GUI_Text(GUI_NextHorizontal(), TextFormat("%.2f", ch->shape.height), colors);

        // Color (RGB)
        GUI_BeginDuplicateBlock();
        GUI_Text(GUI_NextHorizontal(), "color", colors);
        GUI_Text(GUI_NextHorizontal(), TextFormat("r:%d g:%d b:%d a:%d",
            ch->color.r, ch->color.g, ch->color.b, ch->color.a), colors);

        // Movement
        GUI_BeginDuplicateBlock();
        GUI_Text(GUI_NextHorizontal(), "movement.x", colors);
        GUI_Text(GUI_NextHorizontal(), TextFormat("%.3f", ch->movement.x), colors);

        GUI_BeginDuplicateBlock();
        GUI_Text(GUI_NextHorizontal(), "movement.y", colors);
        GUI_Text(GUI_NextHorizontal(), TextFormat("%.3f", ch->movement.y), colors);

        // Animation time
        GUI_BeginDuplicateBlock();
        GUI_Text(GUI_NextHorizontal(), "anim_time", colors);
        GUI_Text(GUI_NextHorizontal(), TextFormat("%.3f", ch->anim_time), colors);

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
        static GUI_Window* win_character_debug = NULL;
        if (GUI_Button(GUI_NextVertical(), "Open Character debug", NULL, window->colors)) {
            if (win_character_debug == NULL || win_character_debug->id == 0) {
                win_character_debug = GUI_MakeWindow(3, "Character debug", (Rectangle){ 20, 20, 300, 100 }, setup->theme.gray, &icons->Dog, false, WIN_character_debug);
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

        GUI_Text(GUI_NextVertical(), "--- Global values ---", window->colors);
        
        // NOTE:
        // GUI_IsPointerOverGui() is not safe to be called by a Window. It requires ALL windows to be processed beforehand. 
        // GUI_Text(GUI_NextVertical(), TextFormat("PointerOverGUI: %d", GUI_IsPointerOverGui()),  window->colors);

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


int main(void) {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);

    InitWindow(DEV_WINDOW_W, DEV_WINDOW_H, TextFormat("%s - %s - %s", BuildTimeFormatted(), __DATE__, GetWorkingDirectory()));
    SetTargetFPS(60);

    while (GetCurrentMonitor() != DEV_TARGET_MONITOR && DEV_TARGET_MONITOR < GetMonitorCount())
        SetWindowMonitor(DEV_TARGET_MONITOR);

    // TODO@dc: review
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
    Game_WindowState win_state = Game_MakeWindowState(&game_state);
    PLAYER_Actions player_actions = PLAYER_MakeActions();

    // Game canvas
    RenderTexture2D game_canvas = LoadRenderTexture(GAME_RES_W, GAME_RES_H);
    SetTextureFilter(game_canvas.texture, TEXTURE_FILTER_POINT);

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
        
        float topbar_height = GUI_CalcDefaultHeightScaled(EGUI_FontType_GUI);
        Rectangle window_limits = (Rectangle){ 
            0,
            topbar_height,
            GetScreenWidth(),
            GetScreenHeight() - topbar_height
        };
        GUI_SetWindowLimits(window_limits);

        GUI_BeginDraw(pointer_style);
        BeginTextureMode(state.buffer);
            ClearBackground(BLANK);
            
            // Top bar
            GUI_TopBar(&player_actions, (Rectangle){ 0, 0, GetScreenWidth(), topbar_height });

            // Window(s)
            float win_third = window_limits.width / 3.0;
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
        EndTextureMode();
        GUI_EndDraw();

        Game_Character *player = Game_GetCurrentCharacter(&game_state);
        Camera2D *camera = &game_state.camera2D;
        camera->target = (Vector2){ player->shape.x, player->shape.y };
        camera->offset = (Vector2){ GAME_RES_HALF_W,  GAME_RES_HALF_H };
        
        // GUI Actions
        if (player_actions.reset_characters) game_state = Game_MakeState();
        if (player_actions.add_character)    Game_AddCharacter(&game_state);  
        if (player_actions.toggle_character) Game_UpdateNextCharacter(&game_state);

        // Scene actions
        if (GUI_IsPointerOverGui() == false) {
            // Update camera
            camera->zoom += ((float)GetMouseWheelMove() * 0.1f);
            if (camera->zoom > 3.0f) camera->zoom = 3.0f;
            else if (camera->zoom < 0.1f) camera->zoom = 0.1f;
        }
        // Keyboard
        player_actions.toggle_character     |= IsKeyPressed(KEY_TAB);
        player_actions.move_down             = IsKeyDown(KEY_DOWN);
        player_actions.move_up               = IsKeyDown(KEY_UP);
        player_actions.move_left             = IsKeyDown(KEY_LEFT);
        player_actions.move_right            = IsKeyDown(KEY_RIGHT);

        // Update character            
        Vector2 move = { 0.0f, 0.0f };
        if (player_actions.move_down)  move.y += 1;
        if (player_actions.move_up)    move.y -= 1;
        if (player_actions.move_left)  move.x -= 1;
        if (player_actions.move_right) move.x += 1;
        
        float dt = GetFrameTime();
        player->movement = move;

        player->shape.x += player->movement.x * CHARACTER_MAX_SPEED * 2 * dt;
        player->shape.y += player->movement.y * CHARACTER_MAX_SPEED * 2 * dt;

        float speed = FloatAbs(player->movement.x);
        if (speed > 0.01f) player->anim_time += dt * speed * 2;
        else player->anim_time = 0;        

        if (IsKeyPressed(KEY_F12)) state.scale += 1.0;
        if (IsKeyPressed(KEY_F11)) state.scale -= 1.0;

        static float ui_opacity = 255.0;
        // 
        // RENDER
        //
        BeginTextureMode(game_canvas);
            ClearBackground(BLANK);

            // Game world
            BeginMode2D(*camera);
                {
                    // Parallax factor: >1.0 = más cerca (se mueve más rápido), <1.0 = más lejos
                    float parallax = 1.2f;

                    // Posiciones base
                    Vector2 p1 = {100, 250};
                    Vector2 p2 = {160, 180};
                    Vector2 p3 = {220, 250};

                    // Ajustar con respecto a la cámara para simular profundidad
                    Vector2 camOffset = camera->target;
                    p1.x += (camOffset.x - camera->offset.x) * (1.0f - 1.0f/parallax);
                    p2.x += (camOffset.x - camera->offset.x) * (1.0f - 1.0f/parallax);
                    p3.x += (camOffset.x - camera->offset.x) * (1.0f - 1.0f/parallax);

                    p1.y += (camOffset.y - camera->offset.y) * (1.0f - 1.0f/parallax);
                    p2.y += (camOffset.y - camera->offset.y) * (1.0f - 1.0f/parallax);
                    p3.y += (camOffset.y - camera->offset.y) * (1.0f - 1.0f/parallax);

                    // Dibujo con efecto "más cercano"
                    DrawLineV(p1, p2, BLACK);
                    DrawLineV(p2, p3, BLACK);
                    DrawLineV(p3, p1, BLACK);
                }

                bool collisions[CHARACTERS];
                float radius = 30.0f;
                Game_UpdateCollisions(&game_state, collisions, radius);

                for (int i = 0; i < game_state.alive_characters; ++i) {
                    Game_Character* c = &game_state.characters[i];
                    
                    Vector2 center = Game_GetCharacterCenter(c);
                    
                    Color ring_color = collisions[i] ? ColorAlpha(WHITE, 0.2) : ColorAlpha(WHITE, 0);
                    DrawRing(center, radius-3, radius, 0, 360, 32, ring_color);
                    
                    float anim_phase = c->anim_time * (c->movement.x < 0 ? 1.0f : -1.0f);
                    DrawCharacter(c->shape, c->movement, anim_phase, c->color);
                }
            EndMode2D();
        EndTextureMode();


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

            float scale_x = FloatCeil((float)GetScreenWidth() / GAME_RES_W);
            float scale_y = FloatCeil((float)GetScreenHeight() / GAME_RES_H);
            camera->target = (Vector2){ player->shape.x * scale_x, player->shape.y * scale_y};
            camera->offset = (Vector2){ GAME_RES_HALF_W * scale_x,  GAME_RES_HALF_H  * scale_y};
            BeginMode2D(*camera);
                {
                    float parallax = 1.4f;

                    static Texture2D arbol = {0};
                    if (arbol.id == 0) arbol = LoadTexture("art/arbol.png");

                    // Configuración
                    int num_arboles = 10;         // cuantos arboles
                    float separacion = 30 * scale_x; // distancia entre arboles
                    Vector2 basePos = { 7 * scale_x, -50 * scale_y };

                    // Calcular desplazamiento relativo a la cámara (efecto parallax)
                    Vector2 camOffset = camera->target;
                    Vector2 camAdjust = {
                        (camOffset.x - camera->offset.x) * (1.0f - 1.0f/parallax),
                        (camOffset.y - camera->offset.y) * (1.0f - 1.0f/parallax)
                    };

                    // --- Horizonte con mismo parallax que los árboles ---
                    Color horizonColor = GRAY;
                    Rectangle horizonRect = {
                        -2000 * scale_x + camAdjust.x,  // desplazado según cámara
                        20 * scale_y + camAdjust.y,   // posición vertical
                        4000 * scale_x,                 // ancho grande
                        300 * scale_y                   // altura
                    };
                    DrawRectangleRec(horizonRect, horizonColor);

                    // Pintar varios árboles en línea horizontal
                    for (int i = 0; i < num_arboles; i++)
                    {
                        Vector2 parallaxPos = {
                            basePos.x + i * separacion + camAdjust.x,
                            basePos.y + camAdjust.y
                        };

                        DrawTextureEx(arbol, parallaxPos, 0, scale_x, WHITE);
                    }
                }



                static Texture2D casaena = {0};
                if (casaena.id == 0) casaena = LoadTexture("art/casaena.png");
                DrawTextureEx(casaena, (Vector2){ 10 * scale_x, -100 * scale_y }, 0, scale_x, WHITE);
            EndMode2D();

            DrawTexturePro(game_canvas.texture,
                    (Rectangle){0, 0, GAME_RES_W, -GAME_RES_H},
                    (Rectangle){0, 0, GetScreenWidth(), GetScreenHeight()},
                    (Vector2){0, 0},
                    0.0f,
                    WHITE
                );
            
            // Draw UI Buffer
            {
                DrawTextureRec(state.buffer.texture, FlipYRec(GetSourceRec(state.buffer.texture)), (Vector2){ 0, 0 }, (Color){ 255, 255, 255, ui_opacity});
            }

            GUI_DrawPointerTrail();
            GUI_DrawPointer();
        EndDrawing();

        first_render = false;
    }

    CloseWindow();
    return 0;
}