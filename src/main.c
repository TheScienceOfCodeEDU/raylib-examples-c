#include "raylib.h"
#include "raymath.h"
#define RAYGUI_IMPLEMENTATION
#include "../lib/raygui/raygui.h"

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

#define board_size  9 // Note: This may be changed but arrays that use this define may require manual updates!
#define row_size    3
#define row_number  3
#define slots 3
u8 board[board_size] = { 0 };

#define empty   (u8)0
#define player1 (u8)1
#define player2 (u8)2

#define MENU_STATE 0
#define GAME_STATE 1

#define START_STATE GAME_STATE

#define DEBUG_CAMERA 0
#define DEBUG_COLLS  0

#define MAX_SNAP_DIST 5.0f

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
        board[i] = 0;
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

Rectangle MoveRectangle(Rectangle rect, Vector2 start, Vector2 end) {
    Vector2 displacement = {
        end.x - start.x,
        end.y - start.y
    };
    
    rect.x += displacement.x;
    rect.y += displacement.y;
    
    return rect;
}

int GetRectangleCornerIndex(Rectangle current_Coll, Vector2 mouse_World2d, float maxDistance) {
    Vector2 corners[4] = {
        { current_Coll.x, current_Coll.y },                                           // 0: Top-left
        { current_Coll.x + current_Coll.width, current_Coll.y },                      // 1: Top-right
        { current_Coll.x, current_Coll.y + current_Coll.height },                     // 2: Bottom-left
        { current_Coll.x + current_Coll.width, current_Coll.y + current_Coll.height } // 3: Bottom-right
    };

    for (int i = 0; i < 4; i++) {
        if (Vector2Distance(mouse_World2d, corners[i]) <= maxDistance) {
            return i;
        }
    }

    return -1;
}

Rectangle MoveRectangleCorner(Rectangle rect, int corner_Idx, Vector2 start, Vector2 end) {
    Vector2 displacement = { end.x - start.x, end.y - start.y };

    switch (corner_Idx) {
        case 0: // Top-left
            rect.x += displacement.x;
            rect.y += displacement.y;
            rect.width -= displacement.x;
            rect.height -= displacement.y;
            break;
        case 1: // Top-right
            rect.y += displacement.y;
            rect.width += displacement.x;
            rect.height -= displacement.y;
            break;
        case 2: // Bottom-left
            rect.x += displacement.x;
            rect.width -= displacement.x;
            rect.height += displacement.y;
            break;
        case 3: // Bottom-right
            rect.width += displacement.x;
            rect.height += displacement.y;
            break;
    }

    return rect;
}

#define QUAD_POINTS 4

// Sample valid Quad: 97.f, 10.f, 142.f, 10.f, 112.f, 41.f, 66.f, 41.f
typedef struct Quad {
    Vector2 points[4]; // Four vertices of the quadrilateral
} Quad;

int GetQuadCornerIndex(Quad quad, Vector2 mouse_World2d, float maxDistance) {
    for (int i = 0; i < 4; i++) {
        if (Vector2Distance(mouse_World2d, quad.points[i]) <= maxDistance) {
            return i;
        }
    }
    return -1;
}

Quad MoveQuad(Quad quad, Vector2 start, Vector2 end) {
    Vector2 displacement = {
        end.x - start.x,
        end.y - start.y
    };
    
    // Move all four corners by the displacement
    for (int i = 0; i < 4; i++) {
        quad.points[i].x += displacement.x;
        quad.points[i].y += displacement.y;
    }
    
    return quad;
  }

Quad MoveQuadCorner(Quad quad, int cornerIndex, Vector2 start, Vector2 end) {
    Vector2 displacement = { end.x - start.x, end.y - start.y };
    
    switch (cornerIndex) {
        case 0: // Top-left
        case 1: // Top-right
        case 2: // Bottom-left
        case 3: // Bottom-right
            quad.points[cornerIndex].x += displacement.x;
            quad.points[cornerIndex].y += displacement.y;
            break;
        default:
            // Invalid corner index, no change
            break;
    }
    
    return quad;
}

void DrawQuad(Quad quad, Color color) {
  // Order matters for drawing
  DrawTriangle(quad.points[2], quad.points[1], quad.points[0], color); // First triangle
  DrawTriangle(quad.points[3], quad.points[2], quad.points[0], color); // Second triangle
}


s32 main() {
  
  InitializeArena(&game_state.arena, Megabytes(1000), (u8 *)mmap(NULL, Megabytes(1000), PROT_READ | PROT_WRITE, 
    MAP_PRIVATE | MAP_ANONYMOUS, -1, 0));

  memset(game_state.arena.Base, 0, game_state.arena.Size); // Touch all pages

  game_state.movements = PushArray(&game_state.arena, 2, Movements);
  
  u8 current_player = player1;
  u8 winner_player = 0;
  
  u8 gameState = START_STATE;

  u8 font_size = 30;
  char *text_win_1 = "Player 1 Wins!";
  char *text_win_2 = "Player 2 Wins!"; 
  
  char *text_title = "Tic Tac Toe";
  char *text_start = "Start Game";
  char *text_exit = "Exit";

  s32 screen_w = 800;
  s32 screen_h = 600;
  s32 rectangle_w = screen_w * 0.1;
  s32 rectangle_h = screen_h * 0.1;

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

  Image artImage = LoadImage("art.png");Rectangle MoveRectangle(Rectangle rect, Vector2 start, Vector2 end);
  if (artImage.data == NULL) {
      artImage = GenImageColor(800, 600, RED);
  }
  Texture artTexture = LoadTextureFromImage(artImage);

  // Art rects
  Rectangle backgroundRect = {0, 0, 242, 118};
  Rectangle player1Rect = {0, 118, 35, 74};
  Rectangle player2Rect = {48, 118, 39, 74};

  Camera2D camera;
    Vector2 offset = { screen_w * 0.5f, screen_h * 0.5f };
    camera.offset = offset;
    camera.rotation = 0.0f;
    camera.zoom = 4.0f;

  Quad board_Coll[board_size] = {
    {97.f, 10.f, 142.f, 10.f, 112.f, 41.f, 66.f, 41.f},
    {143.f, 10.f, 191.f, 10.f, 161.f, 41.f, 112.f, 41.f},
    {192.f, 10.f, 238.f, 10.f, 208.f, 41.f, 162.f, 41.f},

    {65.f, 42.f, 110.f, 42.f, 80.f, 73.f, 34.f, 73.f},
    {111.f, 42.f, 159.f, 42.f, 129.f, 73.f, 80.f, 73.f},
    {160.f, 42.f, 206.f, 42.f, 176.f, 73.f, 130.f, 73.f},
    
    {34.f, 74.f, 79.f, 74.f, 49.f, 105.f, 3.f, 105.f},
    {79.f, 74.f, 127.f, 74.f, 97.f, 105.f, 48.f, 105.f},
    {128.f, 74.f, 174.f, 74.f, 144.f, 104.f, 98.f, 104.f},
  };
  s8 currentColl_Idx = 0;
  bool movingCurrentColl = false;
  s8 movingCurrentCollCorner_Idx = -1;
  Vector2 movingLast_World2d;

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
      Vector2 mouse_World2d = GetScreenToWorld2D(mouse, camera);

      s8 position = -1;
      for (int i = 0; i < board_size; ++i) {
        if (CheckCollisionPointPoly(mouse_World2d, board_Coll[i].points, QUAD_POINTS)) {
          position = i;
          break;
        }
      }

      // Player input
      if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        if (winner_player != 0) {
          winner_player = 0;
          reset_board();
        } else {
          if (position != -1 && board[position] == empty) {
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


    // Update camera
    Vector2 mouse = GetMousePosition();
    Vector2 mouse_World2d = GetScreenToWorld2D(mouse, camera);

    camera.zoom += GetMouseWheelMove() * 1.0f;
    camera.offset = (Vector2){(float)screen_w / 2, (float)screen_h/ 2 };
    camera.target = (Vector2){backgroundRect.width / 2, backgroundRect.height / 2 };

    if (DEBUG_CAMERA) {
      DrawText(TextFormat("%f", camera.zoom), 10, 10, 10, RED);
      DrawText(TextFormat("%f %f", camera.offset.x, camera.offset.y), 10, 20, 10, RED);
      DrawText(TextFormat("%f %f", camera.target.x, camera.target.y), 10, 30, 10, RED);
    }

    //
    // Debug colls update
    //
    bool collideCurrentRect = false;
    if (DEBUG_COLLS) {      
      if (movingCurrentColl) {
        Quad *current_Coll = &board_Coll[currentColl_Idx];

        if (movingCurrentCollCorner_Idx != -1) {
          *current_Coll = MoveQuadCorner(*current_Coll, movingCurrentCollCorner_Idx, movingLast_World2d, mouse_World2d);
        } else {
          *current_Coll = MoveQuad(*current_Coll, movingLast_World2d, mouse_World2d);
        }
        if (IsMouseButtonReleased(MOUSE_RIGHT_BUTTON)) {
          movingCurrentColl = false;          
        }
      } else {  
        
        Quad current_Coll = board_Coll[currentColl_Idx];
        // Update moving corner idx
        movingCurrentCollCorner_Idx = GetQuadCornerIndex(current_Coll, mouse_World2d, MAX_SNAP_DIST);
        // Only, if there is no direct coll against a point
        if (movingCurrentCollCorner_Idx == -1) {
          collideCurrentRect = CheckCollisionPointPoly(mouse_World2d, current_Coll.points, QUAD_POINTS);
        }

        bool anyColl = movingCurrentCollCorner_Idx != -1 || collideCurrentRect;
        if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON) && anyColl) {
          movingCurrentColl = true;
        }
      }
      movingLast_World2d = mouse_World2d;
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
      BeginMode2D(camera);
      DrawTextureRec(artTexture, backgroundRect, Vector2Zero(), WHITE);

      // Manual adjs -> art.aseprite
      const int BACKGROUND_START_X = 106;      
      const int BACKGROUND_START_Y = 39;
      
      const Vector2 player1_figures_World[board_size] = {
        // Row 1
        {BACKGROUND_START_X + 0, BACKGROUND_START_Y},
        {BACKGROUND_START_X + 48, BACKGROUND_START_Y},
        {BACKGROUND_START_X + 96, BACKGROUND_START_Y},
        // Row 2
        {BACKGROUND_START_X + 0 - 33, BACKGROUND_START_Y + 33},
        {BACKGROUND_START_X + 48 - 33, BACKGROUND_START_Y + 33},
        {BACKGROUND_START_X + 96 - 33, BACKGROUND_START_Y + 33},
        // Row 3
        {BACKGROUND_START_X + 0 - 64, BACKGROUND_START_Y + 64},
        {BACKGROUND_START_X + 48 - 64, BACKGROUND_START_Y + 64},
        {BACKGROUND_START_X + 96 - 64, BACKGROUND_START_Y + 64},
      };

      for (s8 i = 0; i < row_size; ++i) {
        for (s8 j = 0; j < row_number; ++j) {
          Vector2 target_World = player1_figures_World[i * row_size + j];
          Vector2 startPlayerFigure_Local, playerFigureTarget_World;
          
          switch (board[i * row_size + j]) {
          case player1:
            startPlayerFigure_Local.x = player1Rect.width / 2;
            startPlayerFigure_Local.y = player1Rect.height + 4;          

            playerFigureTarget_World = Vector2Subtract(target_World, startPlayerFigure_Local);            
            DrawTextureRec(artTexture, player1Rect, playerFigureTarget_World, WHITE);
            break;
          case player2:
            startPlayerFigure_Local.x = player1Rect.width /2 + 2;
            startPlayerFigure_Local.y = player1Rect.height + 2;
            

            playerFigureTarget_World = Vector2Subtract(target_World, startPlayerFigure_Local);
            DrawTextureRec(artTexture, player2Rect, playerFigureTarget_World, WHITE);
            break;
          }
        }
      }

      if (DEBUG_COLLS)
      {
        Quad current_Coll = board_Coll[currentColl_Idx];
        Color dbg_coll_color = collideCurrentRect ? ColorAlpha(RED, 0.5) : ColorAlpha(GREEN, 0.5);

        DrawQuad(current_Coll, dbg_coll_color);
        DrawPixel(current_Coll.points[0].x, current_Coll.points[0].y, movingCurrentCollCorner_Idx == 0 ? YELLOW : dbg_coll_color);
        DrawPixel(current_Coll.points[1].x, current_Coll.points[1].y, movingCurrentCollCorner_Idx == 1 ? YELLOW : dbg_coll_color);
        DrawPixel(current_Coll.points[2].x, current_Coll.points[2].y, movingCurrentCollCorner_Idx == 2 ? YELLOW : dbg_coll_color);
        DrawPixel(current_Coll.points[3].x, current_Coll.points[3].y, movingCurrentCollCorner_Idx == 3 ? YELLOW : dbg_coll_color);
        
        DrawCircle(mouse_World2d.x, mouse_World2d.y, MAX_SNAP_DIST, ColorAlpha(GRAY, 0.1));
      }

      EndMode2D();

      if (DEBUG_COLLS)
      {
        Quad current_Coll = board_Coll[currentColl_Idx];
        const int PANEL_SCREEN_W = 100;
        const int PANEL_SCREEN_X = screen_w - PANEL_SCREEN_W;
        GuiPanel((Rectangle){ PANEL_SCREEN_X, 25, PANEL_SCREEN_W, 140 }, "Panel Info");
        
        GuiLabel((Rectangle){ PANEL_SCREEN_X, 25, PANEL_SCREEN_W, 140 }, TextFormat("x0: %.2f y0: %.2f", current_Coll.points[0].x, current_Coll.points[0].y));
        GuiLabel((Rectangle){ PANEL_SCREEN_X, 25, PANEL_SCREEN_W, 160 }, TextFormat("x1: %.2f y1: %.2f", current_Coll.points[1].x, current_Coll.points[1].y));
        GuiLabel((Rectangle){ PANEL_SCREEN_X, 25, PANEL_SCREEN_W, 180 }, TextFormat("x2: %.2f y2: %.2f", current_Coll.points[2].x, current_Coll.points[2].y));
        GuiLabel((Rectangle){ PANEL_SCREEN_X, 25, PANEL_SCREEN_W, 200 }, TextFormat("x3: %.2f y3: %.2f", current_Coll.points[3].x, current_Coll.points[3].y));
        GuiLabel((Rectangle){ PANEL_SCREEN_X, 25, PANEL_SCREEN_W, 220 }, TextFormat("moving corner: %d", movingCurrentCollCorner_Idx));
        

        if (GuiButton((Rectangle){ PANEL_SCREEN_X, 50, PANEL_SCREEN_W, 10 }, GuiIconText(ICON_ARROW_RIGHT_FILL, "Next"))) {
          currentColl_Idx = (currentColl_Idx + 1) % board_size;
        }
        if (GuiButton((Rectangle){ PANEL_SCREEN_X, 60, PANEL_SCREEN_W, 10 }, GuiIconText(ICON_ARROW_RIGHT_FILL, "Copy values"))) {
          SetClipboardText(TextFormat("%.0f.f, %.0f.f, %.0f.f, %.0f.f, %.0f.f, %.0f.f, %.0f.f, %.0f.f",
            current_Coll.points[0].x, current_Coll.points[0].y,
            current_Coll.points[1].x, current_Coll.points[1].y,
            current_Coll.points[2].x, current_Coll.points[2].y,
            current_Coll.points[3].x, current_Coll.points[3].y
          ));
        }
      }
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


