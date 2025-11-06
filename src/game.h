
#ifndef UNITY_BUILD
 #define UNITY_BUILD 0
 #include <stdio.h> 
 #include "env.h"
 #include <raylib.h>
 #include <raymath.h>
 #include "rayext.h"
 #include "game_structs.h"
 #include "gui_setup.h"
 #include "gui_structs.h" 
 #include "gui.h"
 #include "game_gui.h" 
#endif


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

bool Game_CheckRingCollision(Game_Character* character1, Game_Character* character2, float radius)
{
    Vector2 center1     = RectCenter(character1->shape);
    Vector2 center2     = RectCenter(character2->shape);
    float distance      = Vector2Distance(center1, center2);
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
