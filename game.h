#pragma once

// Board defines
#define board_size 10
#define board_wall_horizontal "-"
#define board_wall_vertical "|"
#define board_wall_corner "+"
#define label_your_board "Your Board"
#define label_enemy_board "Enemy Board"

// Ships
#define ship_count 5
#define carrier 5
#define battleship 4
#define cruiser 3
#define submarine 3
#define destroyer 2

enum ship_types{Carrier, Battleship, Cruiser, Submarine, Destroyer};
const int ship_sizes[] = {carrier, battleship, cruiser, submarine, destroyer};
const char* ship_names[] = {"Carrier", "Battleship", "Cruiser", "Submarine", "Destroyer"};

// Board cell internal states
#define empty 0
#define ship 1
#define shot 2 // Hit water
#define hit 3 // Hit a ship

// Board cell visual states
#define empty_visual "."
#define ship_visual "O"
#define shot_visual "*"
#define hit_visual "x"

const char* board_visual[] = {empty_visual, ship_visual, shot_visual, hit_visual};

// Game boards
typedef int game_board[board_size][board_size];
game_board player_board;
game_board enemy_board;

int remaining_ships_player;
int remaining_ships_enemy;

void init();
void print_board_states();
bool coords_within_board(int x, int y);

bool add_ship(int ship_size, int x, int y, bool horizontal);
bool add_hit(game_board board, int x, int y);
bool add_hit_player(int x, int y);
bool add_hit_enemy(int x, int y);

bool input_position(int* x, int* y);
bool input_position_direction(int* x, int* y, bool* horizontal);

bool add_ship_interactive(int ship_type, int* x_out, int* y_out, bool* horizontal);
void add_all_ships_interactive();
bool add_hit_interactive(int* x_out, int* y_out);