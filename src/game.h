#ifndef UNITY_BUILD 
 #define UNITY_BUILD 0
 #include "common.h"
 #include "gui_setup.h"
 #include "gui_structs.h"
 #include "gui.h"
 #include "game_structs.h"
 #include "game_gui.h"
#endif




void GAME_UpdateNextCharacter()
{
    GAME_State* state   = GAME_CTX.state;
    state->current_character = (state->current_character + 1) % state->alive_characters;
}

void GAME_AddCharacter()
{
    GAME_State* state   = GAME_CTX.state;
    state->alive_characters++;
    if (state->alive_characters > CHARACTERS) state->alive_characters = CHARACTERS;
}

GAME_Character* GAME_GetCurrentCharacter()
{
    GAME_State* state   = GAME_CTX.state;
    return &state->characters[state->current_character];
}

bool GAME_CheckRingCollision(GAME_Character* character1, GAME_Character* character2, float radius)
{
    Vector2 center1     = RectCenter(character1->shape);
    Vector2 center2     = RectCenter(character2->shape);
    float distance      = Vector2Distance(center1, center2);
    return distance < (radius * 2);
}

void GAME_UpdateCollisions(bool collisions[], float radius)
{
    GAME_State* state   = GAME_CTX.state;
    for (int i = 0; i < CHARACTERS; i++) {
        collisions[i] = false;
    }
    
    for (int i = 0; i < state->alive_characters; ++i) {
        for (int j = i + 1; j < state->alive_characters; ++j) {
            if (GAME_CheckRingCollision(&state->characters[i], &state->characters[j], radius)) {
                collisions[i] = true;
                collisions[j] = true;
            }
        }
    }
}
