#define UNITY_BUILD 1
#define IMPLEMENT_ALL 1
#include "common.h"
#include "gui_setup.h"
#include "gui_structs.h"
#include "gui.h"
#include "game_structs.h"
#include "game_gui.h"
#include "game.h"
#include "lab.h"

#define GAME_RES_W          320
#define GAME_RES_H          240
#define GAME_RES_HALF_W     160
#define GAME_RES_HALF_H     120

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
    if (DEV_FULLSCREEN == false)
        SetConfigFlags(FLAG_WINDOW_RESIZABLE);

    InitWindow(DEV_WINDOW_W, DEV_WINDOW_H, TextFormat("%s - %s - %s", BuildTimeFormatted(), __DATE__, GetWorkingDirectory()));
    SetTargetFPS(60);

    int retries = 10;
    while (GetCurrentMonitor() != DEV_TARGET_MONITOR && DEV_TARGET_MONITOR < GetMonitorCount() && --retries > 0)
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
    GUI_State gui_state             = GUI_MakeStateDefault(screen_max);
    GUI_Setup gui_setup             = GUI_MakeSetupDefault();
    GUI_Temp gui_temp               = GUI_MakeTempDefault();
    GUI_Icons icons                 = gui_setup.icon_setup.icons;
    Texture2D wp_voronoi            = GenerateVoronoiTexture((int)screen_max.x, (int)screen_max.y);
    GUI_SetContext(&gui_state, &gui_setup, &gui_temp);

    // PREPARE GAME
    GAME_State game_state           = GAME_MakeState();
    GAME_WindowState win_state      = GAME_MakeWindowState();
    GAME_Temp game_temp             = GAME_MakeTemp();
    Game_SetContext(&game_state, &win_state, &game_temp);

    // Game canvas
    RenderTexture2D game_canvas = LoadRenderTexture(GAME_RES_W, GAME_RES_H);
    SetTextureFilter(game_canvas.texture, TEXTURE_FILTER_POINT);

    SetTargetFPS(60);
    while (!WindowShouldClose()) {
        // RESET TEMP VALUES ONLY 1-FRAME
        GAME_CTX.temp->player_actions   = PLAYER_MakeActions();

        //
        // UPDATE
        //

        //
        // UI
        static EGUI_Pointer pointer_style   = EGUI_Pointer_Default;
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
        
        // Draw UI buffer
        
        BeginTextureMode(gui_state.buffer);
        GUI_BeginDraw(pointer_style);
            ClearBackground(BLANK);
            // Win-manager
            {
                static GUI_Window* win_man = NULL;
                if (win_man == NULL) {
                    win_man = GUI_OpenWindow(1, "WinMan", (Rectangle){ 20, 20, 250, 200 }, gui_setup.theme.gray, &icons.Setup, true, WIN_Winman);
                } else {
                    float win_forth          = window_limits.width / 4.0;
                    win_man->shape.x         = win_forth * 3;
                    win_man->shape.y         = window_limits.y;
                    win_man->shape.width     = win_forth;
                    win_man->shape.height    = window_limits.height;
                }
            }
            GUI_UpdateAndDrawWindows(window_limits);
            GUI_TopBar((Rectangle){ 0, 0, GetScreenWidth(), topbar_height });
        GUI_EndDraw();
        EndTextureMode();

        //
        // CHARACTERS
        GAME_Character *player = GAME_GetCurrentCharacter();
        Camera2D *camera = &game_state.camera2D;
        camera->target = (Vector2){ player->shape.x, player->shape.y };
        camera->offset = (Vector2){ GAME_RES_HALF_W,  GAME_RES_HALF_H };
        
        // GUI Actions
        PLAYER_Actions *player_actions = &GAME_CTX.temp->player_actions;
        if (player_actions->reset_characters)    game_state = GAME_MakeState();
        if (player_actions->add_character)       GAME_AddCharacter(&game_state);  
        if (player_actions->toggle_character)    GAME_UpdateNextCharacter(&game_state);
        if (IsKeyPressed(KEY_F12))              gui_state.scale += 1.0;
        if (IsKeyPressed(KEY_F11))              gui_state.scale -= 1.0;

        if (GUI_IsPointerOverGui() == false) {
            // Update camera
            camera->zoom += ((float)GetMouseWheelMove() * 0.1f);
            if (camera->zoom > 3.0f) camera->zoom = 3.0f;
            else if (camera->zoom < 0.1f) camera->zoom = 0.1f;
        }

        // Character keyboard
        player_actions->toggle_character     |= IsKeyPressed(KEY_TAB);
        player_actions->move_down             = IsKeyDown(KEY_DOWN);
        player_actions->move_up               = IsKeyDown(KEY_UP);
        player_actions->move_left             = IsKeyDown(KEY_LEFT);
        player_actions->move_right            = IsKeyDown(KEY_RIGHT);

        // Update character            
        Vector2 move = { 0.0f, 0.0f };
        if (player_actions->move_down)  move.y += 1;
        if (player_actions->move_up)    move.y -= 1;
        if (player_actions->move_left)  move.x -= 1;
        if (player_actions->move_right) move.x += 1;
        
        float dt = GetFrameTime();

        // Move
        player->movement    = move;
        player->shape.x     += player->movement.x * CHARACTER_MAX_SPEED * 2 * dt;
        player->shape.y     += player->movement.y * CHARACTER_MAX_SPEED * 2 * dt;
        
        // Animate
        float speed = FloatAbs(player->movement.x);
        if (speed > 0.01f)
            player->anim_time   += dt * speed * 2;
        else
            player->anim_time   = 0;        

        
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

                // TODO@dc: Move to update part
                bool collisions[CHARACTERS];
                float radius = 30.0f;
                GAME_UpdateCollisions(collisions, radius);

                for (int i = 0; i < game_state.alive_characters; ++i) {
                    GAME_Character *character   = &game_state.characters[i];
                    Vector2 center              = RectCenter(character->shape);
                    Color ring_color            = collisions[i] ? ColorAlpha(WHITE, 0.2)
                                                                : ColorAlpha(WHITE, 0);
                    float anim_phase            = character->anim_time * (character->movement.x < 0 ? 1.0f : -1.0f);
                    DrawRing(center, radius - 3, radius, 0, 360, 32, ring_color);
                    DrawCharacter(character->shape, character->movement, anim_phase, character->color);
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

            // Wallpaper
            if (win_state.checkbox_value == 1) {
                DrawTextureRec(wp_voronoi, GetSourceRec(wp_voronoi), (Vector2){ 0, 0 }, gui_setup.theme.gray.bg_color_3);
                DrawTexturePro(rain_buffer.texture, GetSourceRec(rain_buffer.texture), MoveAndExtendXY(window_limits, 0, 100), (Vector2){0,0}, 0.0, WHITE);
            }

            // TODO@dc: paint each element and scale them separately instead of using this scale. Work here in plain scene coords, this way we can mix pixelperfect sprites (from textures) with the generated pixel characters in game_canvas.texture.
            // TODO@dc: keep aspect ratio
            // Scaled game screen
            float scale_x = FloatCeil((float)GetScreenWidth() / GAME_RES_W);
            float scale_y = FloatCeil((float)GetScreenHeight() / GAME_RES_H);
            camera->target = (Vector2){ player->shape.x * scale_x, player->shape.y * scale_y};
            camera->offset = (Vector2){ GAME_RES_HALF_W * scale_x,  GAME_RES_HALF_H  * scale_y};
            BeginMode2D(*camera);
                // Scene with paralax
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
            
            // Show UI Buffer
            DrawTextureRec(gui_state.buffer.texture, FlipYRec(GetSourceRec(gui_state.buffer.texture)), (Vector2){ 0, 0 }, WHITE);

            GUI_DrawPointerTrail();
            GUI_DrawPointer();
        EndDrawing();
    }

    CloseWindow();
    return 0;
}