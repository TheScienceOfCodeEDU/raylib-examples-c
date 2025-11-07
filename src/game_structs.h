#ifndef UNITY_BUILD
 #define UNITY_BUILD 0
 #define INCLUDE_GUI
 #include "common.h"
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
} PLAYER_Actions;

PLAYER_Actions PLAYER_MakeActions()
{
    PLAYER_Actions actions = { 0 };
    return actions;
}

typedef struct  {
    Rectangle shape;
    Color color;
    Vector2 movement;
    float anim_time;
} Game_Character;

typedef struct {
    int current_character;
    int alive_characters;
    Game_Character characters[CHARACTERS];
    Camera2D camera2D;
} Game_State;

Game_State Game_MakeState(void)
{
    Game_State state = {
        .current_character = 0,
        .alive_characters  = 2,
        .characters = {
            (Game_Character){  0,  0, 10, 20, RED,    Vector2Zero(), 0 },
            (Game_Character){ 10, 30, 10, 20, BLUE,   Vector2Zero(), 0 },
            (Game_Character){ 50, 60, 10, 20, GREEN,  Vector2Zero(), 0 },
            (Game_Character){ 80, 60, 10, 20, ORANGE, Vector2Zero(), 0 },
        },
        .camera2D = {
            .offset   = (Vector2){ 0, 0 },
            .target   = (Vector2){ 0, 0 },
            .rotation = 0.0f,
            .zoom     = 1.0f
        }
    };

    return state;
}

//
// Win state
//

typedef struct {
    bool checkbox_value;
    bool font_toggle;
    char input_contents[256];
    char input_int_contents[256];
    char input_float_contents[256];
} Game_WindowState;

Game_WindowState Game_MakeWindowState()
{
    Game_WindowState state      = {
        .checkbox_value         = true,
        .font_toggle            = false,
        .input_contents         = {'\0'},
        .input_int_contents     = {'\0'},
        .input_float_contents   = {'\0'}
    };
    return state;
}

// > STATE > CONTEXT
//   STABILITY : █████████░  90%
//   NOTES     : Nothing here

static struct {
    Game_State          *state;
    Game_WindowState    *win_state;
} GAME_CTX = { 0 };

void Game_SetContext(Game_State *state, Game_WindowState *win_state)
{
    GAME_CTX.state      = state;
    GAME_CTX.win_state  = win_state;
}