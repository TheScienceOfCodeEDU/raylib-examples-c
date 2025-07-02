#include "raylib.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h> // For mmap, munmap

typedef int8_t      s8;     // From -127 to  +127
typedef int16_t     s16;    
typedef int32_t     s32;
typedef int64_t     s64;
typedef uint8_t     u8;     // From 0 to 255
typedef uint16_t    u16;
typedef uint32_t    u32;
typedef uint64_t    u64;
typedef float       r32;    // Generally used
typedef double      r64;
typedef s8          b8;     // boolean 8 bits
typedef s32         b32;    // boolean 32 bits

#define false       0
#define true        1
#define Pi32        3.14159265359

#define local_persist   static
#define global_persist  static
#define function        static

#define board_size  9
#define row_size    3
#define row_number  3
#define slots 3
u8 board[board_size] = { 0 };

#define empty   (u8)0
#define player1 (u8)1
#define player2 (u8)2

#define MENU_STATE 0
#define GAME_STATE 1

/* Memory */

typedef struct
{
	u32 Size;
	u8 *Base;
	u32 Used;
} memory_arena;

function void
InitializeArena(memory_arena *Arena, u32 Size, u8 *Base)
{
	Arena->Size = Size;
	Arena->Base = Base;
	Arena->Used = 0;
}

#define PushStruct(Arena, Type) (Type *) PushSize_(Arena, sizeof(Type))
#define PushArray(Arena, Count, Type) (Type *) PushSize_(Arena, sizeof(Type) * Count)
void*
PushSize_(memory_arena* Arena, u32 Size)
{
	// Assert
	u8 *Base = Arena->Base + Arena->Used;
	Arena->Used += Size;
	return Base;
}

#define Kilobytes(Value) ((Value)*1024)
#define Megabytes(Value) (Kilobytes(Value)*1024)
#define Gigabytes(Value) (Megabytes(Value)*1024)
/* End Memory */



struct {
  s8 positions [slots];
  s8 index;
} typedef Movements;


struct {
  memory_arena arena;
  Movements *movements;
} typedef GameState;

GameState game_state;
//Movements previous_movements [2] = { 0 };

Movements make_movements()
{
  Movements result = { {-1,-1,-1}, 0};
  return result;
}

void store_movements_and_update(Movements *player, s8 new_position)
{  
  s8 prev_position = player->positions[player->index];
  if (prev_position != -1) {
    board[prev_position] = empty;
  }

  player->positions[player->index] = new_position;
  player->index = (player->index + 1) % slots; 
}

function void print_board()
{
    for (u8 i = 0; i < board_size; ++i) {
        if (i % 3 == 0) {
            printf("\n");
        }

        switch (board[i]) {
        case empty:
            printf("."); break;
        case player1:
            printf("X"); break;
        case player2:
            printf("O"); break;
        }        
    }
    printf("\n\n");
}

function void reset_board()
{
    for (u8 i = 0; i < board_size; ++i) {
        board[i] = empty;
    }
    game_state.movements[0] = make_movements();
    game_state.movements[1] = make_movements();
}


function b8 winner(s8 current_player)
{   
    for (s8 i = 0; i < row_number; ++i) {
        if (board[i * row_size] == current_player && 
            board[i * row_size + 1] == current_player && 
            board[i * row_size + 2] == current_player) {
            return true;
        }
    }

    for (s8 i = 0; i < row_number; ++i) {
        if (board[i] == current_player && 
            board[i + row_size] == current_player && 
            board[i + row_size * 2] == current_player) {
            return true;
        }
    }
    
    if (board[0] == current_player && 
        board[4] == current_player && 
        board[8] == current_player) {
        return true;
    }
    if (board[2] == current_player && 
        board[4] == current_player && 
        board[6] == current_player) {
        return true;
    }
    return false;
}

void copy(s32 size, void *obj, void *target)
{
    for (s32 i = 0; i < size; ++i) {
        u8 *byte_target = ((u8 *)target) + i;
        u8 *byte_obj = ((u8 *)obj) + i;
        *byte_target = *byte_obj;
    }
}



s32 main() {
  
  InitializeArena(&game_state.arena, Megabytes(1000), (u8 *)mmap(NULL, Megabytes(1000), PROT_READ | PROT_WRITE, 
    MAP_PRIVATE | MAP_ANONYMOUS, -1, 0));

  memset(game_state.arena.Base, 0, game_state.arena.Size); // Touch all pages

  game_state.movements = PushArray(&game_state.arena, 2, Movements);
  
  u8 current_player = player1;
  u8 winner_player = 0;
  
  u8 gameState = MENU_STATE;

  u8 font_size = 30;
  char *text_win_1 = "Player 1 Wins!";
  char *text_win_2 = "Player 2 Wins!"; 
  
  char *text_title = "Tic Tac Toe";
  char *text_start = "Start Game";
  char *text_exit = "Exit";

  s32 screen_w = 800;
  s32 screen_h = 600;
  s32 rectangle_w = screen_w / row_size;
  s32 rectangle_h = screen_h / row_number;

   // Define button dimensions and positions

  u8 buttonWidth = 200;
  u8 buttonHeight = 50;
  u16 startButtonX = screen_w/2 - buttonWidth/2;
  u16 startButtonY = screen_h/2 - 30;
  u16 exitButtonX = screen_w/2 - buttonWidth/2;
  u16 exitButtonY = screen_h/2 + 40;

  Rectangle startButtonRect = { startButtonX, startButtonY, buttonWidth, buttonHeight };
  Rectangle exitButtonRect = { exitButtonX, exitButtonY, buttonWidth, buttonHeight };

  reset_board();

  InitWindow(screen_w, screen_h, "raylib basic window");
  SetTargetFPS(60);
  while (!WindowShouldClose()) {
    Vector2 mousePoint = GetMousePosition();

    if(gameState == MENU_STATE){
      b8 startButtonHover = CheckCollisionPointRec(mousePoint, startButtonRect);
      b8 exitButtonHover = CheckCollisionPointRec(mousePoint, exitButtonRect);

      if(startButtonHover && IsMouseButtonReleased(MOUSE_LEFT_BUTTON)){
        gameState = GAME_STATE;
        reset_board();
        current_player = player1;
        winner_player = 0;
      }
      if(exitButtonHover && IsMouseButtonReleased(MOUSE_LEFT_BUTTON)){
        CloseWindow();
        return 0;
      }
    }
    else if (gameState == GAME_STATE) {
      Vector2 mouse = GetMousePosition();
      s8 hover_row = mouse.y / rectangle_h;
      s8 hover_col = mouse.x / rectangle_w;
    
      s8 position = hover_col + hover_row * row_size;

      
      if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        if (winner_player != 0) {
          winner_player = 0;
          reset_board();
        } else {
          if (board[position] == empty) {
            // Update board
            board[position] = current_player;

            store_movements_and_update(&game_state.movements[current_player - 1], position);

            // Winner?
            if (winner(current_player)) {
              winner_player = current_player;
            } else{
              // Update player
              if (current_player == player1)
                  current_player = player2;
              else
                  current_player = player1;

            }       
          }
        }
      }
    }

    BeginDrawing();
    ClearBackground(RAYWHITE);

    if(gameState == MENU_STATE){
      u8 titleWidth = MeasureText(text_title, 40);
      DrawText(text_title, screen_w/2 - titleWidth/2, screen_h/4, 40, BLACK);

      Color startButtonColor = CheckCollisionPointRec(mousePoint, startButtonRect) ? SKYBLUE : LIGHTGRAY;
      DrawRectangleRec(startButtonRect, startButtonColor);
      DrawRectangleLinesEx(startButtonRect, 2, BLACK);

      u8 startTextWidth = MeasureText(text_start, 20);
      DrawText(text_start, startButtonX + buttonWidth/2 - startTextWidth/2, startButtonY + buttonHeight/2 - 10, 20, BLACK);

      Color exitButtonColor = CheckCollisionPointRec(mousePoint, exitButtonRect) ? SKYBLUE : LIGHTGRAY;
      DrawRectangleRec(exitButtonRect, exitButtonColor);
      DrawRectangleLinesEx(exitButtonRect, 2, BLACK);

      u8 exitTextWidth = MeasureText(text_exit, 20);
      DrawText(text_exit, exitButtonX + buttonWidth/2 - exitTextWidth/2, exitButtonY + buttonHeight/2 - 10, 20, BLACK);
    }
    else if (gameState == GAME_STATE) {
      Vector2 mouse = GetMousePosition();
      s8 hover_row = mouse.y / rectangle_h;
      s8 hover_col = mouse.x / rectangle_w;

      // Board lines
      for (s8 i = 0; i < row_size; ++i) {
        for (s8 j = 0; j < row_number; ++j) {
          DrawRectangleLines(j * rectangle_w, i * rectangle_h, rectangle_w, rectangle_h, LIGHTGRAY);

          switch (board[i * row_size + j]) {
          case player1: 
            DrawRectangle(j * rectangle_w, i * rectangle_h, rectangle_w, rectangle_h, RED); 
            break;
          case player2: 
            DrawRectangle(j * rectangle_w, i * rectangle_h, rectangle_w, rectangle_h, BLUE);
            break;
          }
        }
      }
      DrawRectangle(hover_col * rectangle_w, hover_row * rectangle_h, rectangle_w, rectangle_h, ColorAlpha(BLACK, 0.1));

    }

    if (winner_player == 0) {
      char *turn_text = current_player == player1 ? "Player 1's Turn (Red)" : "Player 2's Turn (Blue)";
      u8 turnTextSize = MeasureText(turn_text, 20);
      DrawText(turn_text, screen_w/2 - turnTextSize/2, 20, 20, BLACK);

    }

    if (winner_player != 0) {
      char *current_text = winner_player == 1 ? text_win_1 : text_win_2;
      u8 text_size = MeasureText(current_text, font_size);

      DrawText(current_text, screen_w / 2 - text_size / 2 - 2, screen_h / 2 + 2, font_size, BLACK);
      DrawText(current_text, screen_w / 2 - text_size / 2, screen_h / 2, font_size, GREEN);
    }
    
    EndDrawing();
  }
  CloseWindow();
  return 0;
}



/*
stack  -->
heap    ---> dinámico new

List<int> 

[ooooooooooooo      oooooooooooooooooooooo                                                              ]             
*/


