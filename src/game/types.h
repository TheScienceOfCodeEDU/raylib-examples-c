#pragma once
#ifndef NON_EDITOR_BUILD
#define NON_EDITOR_BUILD 0
#include "../common.h"
#endif

//
// CORE
//

typedef struct {
    Texture2D texture;
    Vector2 position;
    float parallax;      // Parallax factor (1.0f = camera locked)
    float scale;
    Color tint;
} Game_Element;

Game_Element Game_MakeElement(const char *path, Vector2 position, float parallax, float scale, Color tint)
{
    Game_Element element = {
        .texture    = LoadTexture(path),
        .position   = position,
        .parallax   = parallax,
        .scale      = scale,
        .tint       = tint
    };
    return element;
}

//
// GAME ITSELF
//

#define CHARACTERS              4
#define CHARACTER_MAX_SPEED     32

typedef struct {
    bool reset_characters;
    bool add_character;
    bool toggle_character;
    bool move_up;
    bool move_down;
    bool move_left;
    bool move_right;
} GAME_Actions;

GAME_Actions GAME_MakeActions()
{
    GAME_Actions actions = { 0 };
    return actions;
}

typedef struct  {
    Rectangle shape;
    Color color;
    Vector2 movement;
    float anim_time;
} GAME_Character;

typedef struct {
    int             current_character;
    int             alive_characters;
    Camera2D        camera2D;
    GAME_Character  characters[CHARACTERS];
} GAME_State;

GAME_State GAME_MakeState(void)
{
    GAME_State state = {
        .current_character = 0,
        .alive_characters  = 2,
        .camera2D = {
            .offset   = (Vector2){ 0, 0 },
            .target   = (Vector2){ 0, 0 },
            .rotation = 0.0f,
            .zoom     = 1.0f
        },
        .characters = {
            (GAME_Character){  0,  0, 10, 20, RED,    Vector2Zero(), 0 },
            (GAME_Character){ 10, 30, 10, 20, BLUE,   Vector2Zero(), 0 },
            (GAME_Character){ 50, 60, 10, 20, GREEN,  Vector2Zero(), 0 },
            (GAME_Character){ 80, 60, 10, 20, ORANGE, Vector2Zero(), 0 },
        }
    };

    return state;
}

// > GAME GUI State

typedef struct {
    bool checkbox_value;
    bool font_toggle;
    char input_contents[256];
    char input_int_contents[256];
    char input_float_contents[256];
} GAME_WindowState;

GAME_WindowState GAME_MakeWindowState()
{
    GAME_WindowState state      = {
        .checkbox_value         = true,
        .font_toggle            = false,
        .input_contents         = {'\0'},
        .input_int_contents     = {'\0'},
        .input_float_contents   = {'\0'}
    };
    return state;
}

typedef struct {
    GAME_Actions  player_actions;
} GAME_Temp;

GAME_Temp GAME_MakeTemp()
{
    GAME_Temp temp = {
        .player_actions     = {0}
    };
    return temp;
}

// > CONTEXT

static struct {
    GAME_State          *state;
    GAME_WindowState    *win_state;
    GAME_Temp           *temp;
} GAME_CTX = { 0 };

void GAME_SetContext(GAME_State *state, GAME_WindowState *win_state, GAME_Temp *temp)
{
    GAME_CTX.state      = state;
    GAME_CTX.win_state  = win_state;
    GAME_CTX.temp       = temp;
}
