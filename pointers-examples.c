#include <stdint.h>
#include <stdio.h>
/*
[
    int de 32 bits
    float de 32 bits
    float de 32 bits
    char de 8 bits
    char de 8 bits
    char de 8 bits
    char de 8 bits
    char de 8 bits
    char de 8 bits
    char de 8 bits
    ...256
]*/

struct {
    int id; //4
    float health; //4
    float experience; //4
    //char name [256];
} typedef Player;

struct {
    float health; //4
    int id; //4
    //char name [256];
} typedef Enemy;

Player make_player(int id, float health, float experience/*, char name [256]*/)
{
    Player p = {id, health, experience};
    return p;
}

void print_player(Player *player1)
{
    printf("id: %d health: %f experience: %f\n", player1->id, player1->health, player1->experience);
    //printf("peso bytes: %d \n", sizeof(Player));
    //printf("peso bytes *: %d \n", sizeof(p1));
}

typedef uint8_t u8;
void print_all_players(Player player [], int size) 
{
    for (int i = 0; i < size; ++i) {
        //Player *pactual = player + i;
        printf("id: %d health: %f experience: %f\n", player->id, player->health, player->experience);
        player++;
    }
    
    //printf("peso bytes: %d \n", sizeof(Player));
    //printf("peso bytes *: %d \n", sizeof(p1));
}

typedef uint8_t byte;

void print_all_health_players(void *obj_health, int size, int size_struct)
{
    for (int i = 0; i < size; ++i) {
        float *current_health = (float *)((byte *)obj_health + size_struct * i); 
        //float *current_health = (float*) ((Player *)obj_health + i);
        printf("health: %f\n", *current_health); 
    }
    
    //printf("peso bytes: %d \n", sizeof(Player));
    //printf("peso bytes *: %d \n", sizeof(p1));
}

int main () {
    /*Player player1 = make_player(1, 100.0f, 5.0f); 
    Player player2 = make_player(2, 200.0f, 1.0f);
    Player player3 = make_player(3, 300.0f, 2.0f);
    Player player4 = make_player(4, 400.0f, 3.0f);
    Player player5 = make_player(5, 500.0f, 4.0f);*/
    //Player *p1; #64 bits address
    
    Player players [5] =  // Player *players
    {
        make_player(1, 100.0f, 5.0f),
        make_player(2, 200.0f, 1.0f),
        make_player(3, 300.0f, 2.0f),
        make_player(4, 400.0f, 3.0f),
        make_player(5, 500.0f, 4.0f)
    };

#define size_array(a, b) sizeof(a)/sizeof(b)


    Enemy enemy = { -100, 0 };
    Enemy *pene = &enemy;
    Enemy enemy2 = enemy;
    Enemy enemy3 = *pene;

    pene->health = 10000;

    print_player(players);
    print_all_players(players, size_array(players, Player));
    print_all_health_players(&players->health, size_array(players, Player), sizeof(Player));
    print_all_health_players(&enemy.health, 1, sizeof(Enemy));
    //printf("id: %d health: %f experience: %f\n", player1.id, player1.health, player1.experience);

    return 0;
}
