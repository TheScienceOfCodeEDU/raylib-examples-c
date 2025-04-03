#include "raylib.h"

#include <stdint.h>
#include <stdio.h>

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
u8 board[board_size] = { 0 };

#define empty   (u8)0
#define player1 (u8)1
#define player2 (u8)2 

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
  u8 current_player = player1;
  u8 winner_player = 0;  
  u8 win_message_size = 220;  


  s32 screen_w = 800;
  s32 screen_h = 600;
  s32 rectangle_w = screen_w / row_size;
  s32 rectangle_h = screen_h / row_number;

  InitWindow(screen_w, screen_h, "raylib basic window");
  SetTargetFPS(60);
  while (!WindowShouldClose()) {

    Vector2 mouse = GetMousePosition();
    s8 hover_row = mouse.y / rectangle_h;
    s8 hover_col = mouse.x / rectangle_w;
    
    s8 position = hover_col + hover_row * row_size;

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
      winner_player = 0;
      if (board[position] == empty) {
        // Update board
        board[position] = current_player;

        // Winner?
        if (winner(current_player)) {
          winner_player = current_player;
          reset_board();
        }

        // Update player
        if (current_player == player1)
            current_player = player2;
        else
            current_player = player1;
      }
    }


    BeginDrawing();
    ClearBackground(RAYWHITE);

    // Board lines
    for (s8 i = 0; i < row_size; ++i) {
      for (s8 j = 0; j < row_number; ++j) {
        DrawRectangleLines(j * rectangle_w, i * rectangle_h, rectangle_w, rectangle_h, RED);

        switch (board[i * row_size + j]) {
        case player1: 
          DrawRectangle(j * rectangle_w, i * rectangle_h, rectangle_w, rectangle_h, BLUE); 
          break;
        case player2: 
          DrawRectangle(j * rectangle_w, i * rectangle_h, rectangle_w, rectangle_h, GREEN);
          break;
        }
      }
    }
    DrawRectangle(hover_col * rectangle_w, hover_row * rectangle_h, rectangle_w, rectangle_h, ColorAlpha(BLACK, 0.1));


    DrawText(TextFormat("Mouse: %f %f Board: %i %i", mouse.x, mouse.y, hover_col, hover_row), 20, 20, 20, BLACK);
    DrawText(TextFormat("Mouse: %f %f Board: %i %i", mouse.x, mouse.y, hover_col, hover_row), 20, 20, 20, BLACK);


    if (winner_player != 0) {
      DrawText((winner_player == player1) ? "Player 1 Wins!" : "Player 2 Wins!", screen_w / 2 - win_message_size/2, screen_h / 2, 30, DARKGREEN);
    }
    
    EndDrawing();
  }
  CloseWindow();
  return 0;
}