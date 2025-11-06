#ifndef UNITY_BUILD
  #include "gui.h"
#endif

#define CHARACTERS              4
#define CHARACTER_MAX_SPEED     32

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

Game_State Game_MakeState()
{
    Game_State state = {
        0,
        2,
        (Game_Character){ 0, 0, 10, 20, RED, Vector2Zero(), 0},
        (Game_Character){ 10, 30, 10, 20, BLUE, Vector2Zero(), 0},
        (Game_Character){ 50, 60, 10, 20, GREEN, Vector2Zero(), 0},
        (Game_Character){ 80, 60, 10, 20, ORANGE, Vector2Zero(), 0},
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
        character->shape.x + character->shape.width / 2.0f,
        character->shape.y + character->shape.height / 2.0f
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
