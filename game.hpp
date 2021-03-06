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

typedef int game_board[board_size][board_size];

class Game {
  private:
    static const int ship_sizes[];
    static const char* ship_names[];
    static const char* board_visual[];

    int remaining_ships_player;
    int remaining_ships_enemy;

    bool input_position_raw(int* x, int* y);

  public:
    game_board player_board;
    game_board enemy_board;

    Game(); // Initialize game variables
    void print_board_states() const; // Print the entire state of both boards
    bool coords_within_board(int x, int y) const; // Check if the given coordinates are within a board

    bool add_ship(int ship_size, int x, int y, bool horizontal); // Add a ship to the player's board
    bool add_hit_player(int x, int y); // Add a hit to the player's board
    bool add_hit_enemy(int x, int y, int hit_type); // Add a hit to the enemy's board
    bool check_victory() const; // Check if the player has won
    bool check_loss() const; // Check if the player has lost

    void clear_stdin() const; // Clear current stdin buffer
    bool input_position(int* x, int* y); // Read position from stdio
    bool input_position_direction(int* x, int* y, bool* horizontal); // Read position and direction from stdio

    bool add_ship_interactive(int ship_type, int* x_out, int* y_out, bool* horizontal); // Interactively add a ship to the player's board
    void add_all_ships_interactive(); // Add all ships with interactive add
    bool add_hit_interactive(int* x_out, int* y_out); // Interactively add a hit to the enemy's board
};
