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

// 345012678 POSITION
// 678345012 CORRESPONDING KEYNUMBER (based on 0)
// 678
// 345
// 012
u8 corresponding_keynumber[9] = {6,7,8,3,4,5,0,1,2};

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

s32 main () {

    u8 current_player = player1;
    while (true) {
        // Print
        print_board();
    
        // Keys
        u32 keynumber;
        scanf_s("%d", &keynumber);
        u8 position = corresponding_keynumber[keynumber - 1];

        // Non empty
        if (board[position] != empty) {
            printf("Occupied position\n");
            continue;
        }
        
        // Update board
        board[position] = current_player;
        

        // Winner?
        if (winner(current_player)) {
            print_board();
            printf("Wins %u \n", current_player);
            break;
        }

        // Update player
        if (current_player == player1)
            current_player = player2;
        else
            current_player = player1;

    }
    

    return 0;
}
