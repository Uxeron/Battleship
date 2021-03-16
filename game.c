#include <stdio.h>
#include <string.h>
#include <stdbool.h>

// Board defines
#define board_size 10
#define board_wall_horizontal "-"
#define board_wall_vertical "|"
#define board_wall_corner "+"
#define label_your_board "Your Board"
#define label_enemy_board "Enemy Board"

// Ship sizes
#define carrier 5
#define battleship 4
#define cruiser 3
#define submarine 3
#define destroyer 2

// Board cell internal states
#define empty 0
#define ship 1
#define shot 2
#define hit 3

// Board cell visual states
#define empty_visual "."
#define ship_visual "#"
#define shot_visual "*"
#define hit_visual "x"
#define target_visual "X"

const char* board_visual[] = {empty_visual, ship_visual, shot_visual, hit_visual};

// Game boards
int player_board[board_size][board_size];
int enemy_board[board_size][board_size];


void init() {
    // Initialize both boards with 0's
    memset(player_board, 0, sizeof(player_board));
    memset(enemy_board, 0, sizeof(enemy_board));
}

void print_board_states() {
    // Print labels
    int padding = ((board_size*2 + 3) - strlen(label_your_board)) / 2;
    printf("%*c", padding, ' ');
    printf(label_your_board);
    printf("%*c", padding, ' ');

    printf("  ");

    padding = ((board_size*2 + 3) - strlen(label_enemy_board)) / 2;
    printf("%*c", padding, ' ');
    printf(label_enemy_board);
    printf("%*c", padding, ' ');

    printf("\n");

    // Draw board wall tops
    printf(board_wall_corner);
    for (int i = 0; i < board_size * 2 + 1; i++) printf(board_wall_horizontal);
    printf(board_wall_corner);

    printf("  ");

    printf(board_wall_corner);
    for (int i = 0; i < board_size * 2 + 1; i++) printf(board_wall_horizontal);
    printf(board_wall_corner);

    printf("\n");

    // Draw both boards with vertical walls at the start and end
    for (int y = 0; y < board_size; y++) {
        printf(board_wall_vertical);
        printf(" ");
        for (int x = 0; x < board_size; x++) {
            printf("%s ", board_visual[player_board[x][y]]);
        }
        printf(board_wall_vertical);

        printf("  ");

        printf(board_wall_vertical);
        printf(" ");
        for (int x = 0; x < board_size; x++) {
            printf("%s ", board_visual[enemy_board[x][y]]);
        }
        printf(board_wall_vertical);

        printf("\n");
    }

    // Draw board wall bottoms
    printf(board_wall_corner);
    for (int i = 0; i < board_size * 2 + 1; i++) printf(board_wall_horizontal);
    printf(board_wall_corner);

    printf("  ");

    printf(board_wall_corner);
    for (int i = 0; i < board_size * 2 + 1; i++) printf(board_wall_horizontal);
    printf(board_wall_corner);

    printf("\n");
}

bool add_ship(int ship_size, int x, int y, bool horizontal) {
    // Check if the xy coordinates are within the board
    if (x >= board_size)
        return false;
    
    if (y >= board_size)
        return false;
    
    // Check if the ship fits in the board
    if (horizontal)
        if (x + ship_size > board_size)
            return false;
    else
        if (y + ship_size > board_size)
            return false;

    // Try to add the ship
    if (horizontal) {
        // Check if the cells aren't occupied
        for (int i = x; i < x + ship_size; i++) {
            if (player_board[i][y] != empty)
                return false;
        }

        // Add the ship
        for (int i = x; i < x + ship_size; i++) {
            player_board[i][y] = ship;
        }
    } else {
        // Check if the cells aren't occupied
        for (int i = y; i < y + ship_size; i++) {
            if (player_board[x][i] != empty)
                return false;
        }

        // Add the ship
        for (int i = y; i < y + ship_size; i++) {
            player_board[x][i] = ship;
        }
    }

    return true;
}

int main() {
    init();
    add_ship(carrier, 6, 0, false);
    add_ship(battleship, 1, 0, true);

    print_board_states();

    return 0;
}
