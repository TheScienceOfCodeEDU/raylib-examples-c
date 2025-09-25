#define UNITY_BUILD 1
#include "raylib.h"
#include "raymath.h"
#include "env.h"
#include "str.h"
#include "rayext.h"
#include "gui.h"


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

    actions->reset_characters    = GUI_Button("Reset", (Rectangle) { button_w * 0, 0, button_w, button_h }, gui, &gui->icons.New, gui->theme.red);
    actions->add_character       = GUI_Button("Add", (Rectangle) { button_w * 1, 0, button_w, button_h }, gui, &gui->icons.Open, gui->theme.gray);
    actions->toggle_character    = GUI_Button("Change", (Rectangle) { button_w * 2, 0, button_w, button_h }, gui, &gui->icons.Save, gui->theme.gray);
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

    while (!WindowShouldClose()) {
        //
        // UPDATE
        //

        // UI
        gui.default_height          = GUI_CalcDefaultScaledHeight(&gui);
        gui.focus_state_current     = GUI_Focus_Available;

        Rectangle mouse_limits = (Rectangle) {
            0,
            0,
            GetScreenWidth(),
            GetScreenHeight()
        };
        gui.mouse_current = LimitVector2Rect(GetMousePosition(), mouse_limits);        

        Vector2 mouse_shape = (Vector2){
            gui.mouse_current.x - (mouse_texture.width * gui.scale * 0.5f),
            gui.mouse_current.y - (mouse_texture.height * gui.scale * 0.5f),
        };

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            gui.control_focus_id = 0;
        }

        Rectangle window_limits = (Rectangle){ 
            0,
            gui.default_height,
            GetScreenWidth(),
            GetScreenHeight() - gui.default_height // Minus GUI_TopBar size
        };

        BeginTextureMode(buffer);
            ClearBackground(BLANK);
            
            // Top bar
            GUI_TopBar(&gui, &player_actions, (Rectangle){ 0, 0, GetScreenWidth(), gui.default_height });

            // Data
            static bool checkbox_value = 0;
            static char textbox_contents[256] = "hello\0";
            static char textbox2_contents[256] = "world\0";

            // Window
            static Rectangle window = { 20, 20, 250, 200 };
    
            const int ELEMENTS = 4;
            window.height =(gui.default_height + gui.theme.border * gui.scale) * (ELEMENTS + 1);

            GUI_Window(1, "Window title", &gui, &window, window_limits, gui.theme.gray);
            {
                // Window contents
                Rectangle window_workspace = GUI_WindowWorkspace(window, &gui);
                GUI_ResetLayout();

                // 1st textbox
                GUI_BeginBlock(window_workspace.width / 2, gui.default_height, &window_workspace);
                GUI_Label("Name", 
                    RelativeToRect(GUI_NextHorizontal(), window_workspace), &gui, gui.theme.gray);
                GUI_TextBox(2, textbox_contents,
                    RelativeToRect(GUI_NextHorizontal(), window_workspace), &gui, gui.theme.gray);
                
                // Other elements
                GUI_BeginBlock(window_workspace.width, gui.default_height, &window_workspace);
                GUI_TextBox (3, textbox2_contents,
                    RelativeToRect(GUI_NextVertical(), window_workspace), &gui, gui.theme.gray);
                GUI_CheckBox(4, &checkbox_value, "On", "Off",
                    RelativeToRect(GUI_NextVertical(), window_workspace), &gui, gui.theme.red);
            }

        #if DEV_DEBUG_GUI
            static Rectangle win_debug = { 20, 220, 350, 200 };
            float win_third     = window_limits.width / 3.0;
            win_debug.x         = win_third * 2;
            win_debug.y         = window_limits.y;
            win_debug.width     = win_third;
            win_debug.height    = window_limits.height;
            
            GUI_Window(4, "Kairos Debug", &gui, &win_debug, window_limits, gui.theme.gray);
            {
                // Window contents
                Rectangle window_workspace = GUI_WindowWorkspace(win_debug, &gui);

                GUI_BeginVertical(gui.default_height);
                GUI_BeginHorizontal(window_workspace.width);
                GUI_Label(textbox_contents, RelativeToRect(GUI_NextVertical(), window_workspace), &gui, gui.theme.gray);
            }

            static Rectangle win_layouts = { 20, 220, 350, 200 };
            win_layouts.x         = win_third;
            win_layouts.y         = window_limits.y;
            win_layouts.width     = win_third;
            win_layouts.height    = window_limits.height;
            GUI_Window(500, "Kairos Layouts", &gui, &win_layouts, window_limits, gui.theme.gray);
            {
                // Window contents
                Rectangle window_workspace = GUI_WindowWorkspace(win_layouts, &gui);
                GUI_ResetLayout();

                // First block
                // Reset width for elements of full width
                // Reset horizontal for elements of default height
                // 2 verticals of full width
                GUI_BeginBlock(window_workspace.width, gui.default_height, &window_workspace);
                DrawDebugRect(RelativeToRect(GUI_NextVertical(), window_workspace), RED);
                DrawDebugRect(RelativeToRect(GUI_NextVertical(), window_workspace), YELLOW);
                // Second block
                // 3 horizontals of 1/3 of the available space
                GUI_BeginBlock(window_workspace.width / 3, gui.default_height, &window_workspace);
                DrawDebugRect(RelativeToRect(GUI_NextHorizontal(), window_workspace), DARKPURPLE);
                DrawDebugRect(RelativeToRect(GUI_NextHorizontal(), window_workspace), DARKBROWN);
                DrawDebugRect(RelativeToRect(GUI_NextHorizontal(), window_workspace), DARKGREEN);
                
                // Prepare for a new block with 5 elements per row
                GUI_BeginBlock(window_workspace.width / 5, -gui.default_height, &window_workspace);
                DrawDebugRect(RelativeToRect(GUI_NextHorizontal(), window_workspace), BLACK);
                DrawDebugRect(RelativeToRect(GUI_NextHorizontal(), window_workspace), BLACK);
                DrawDebugRect(RelativeToRect(GUI_NextHorizontal(), window_workspace), BLACK);
                DrawDebugRect(RelativeToRect(GUI_NextHorizontal(), window_workspace), BLACK);
                DrawDebugRect(RelativeToRect(GUI_NextHorizontal(), window_workspace), BLACK);
                
                GUI_BeginBlock(window_workspace.width / 2, gui.default_height, &window_workspace);
                DrawDebugRect(RelativeToRect(GUI_NextHorizontal(), window_workspace), RED);
                DrawDebugRect(RelativeToRect(GUI_NextHorizontal(), window_workspace), BLUE);
            }
        #endif
            
            DrawText(TextFormat("focus_win: %d", gui.window_focus_id), 10, 70, 20, BLACK);
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

        static float ui_opacity = 255.0;

        // 
        // RENDER
        //

        // Draw
        BeginDrawing();
            ClearBackground(RAYWHITE);

            // Game world
            BeginMode2D(camera);
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
            Rectangle sourceRec = { 0, 0, (float)buffer.texture.width, - (float)buffer.texture.height };
            DrawTextureRec(buffer.texture, sourceRec, (Vector2){ 0, 0 }, (Color){ 255, 255, 255, ui_opacity});
        EndDrawing();
    }

    CloseWindow();
    return 0;
}