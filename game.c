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

void init() {
    // Initialize both boards with 0's
    memset(player_board, 0, sizeof(player_board));
    memset(enemy_board, 0, sizeof(enemy_board));

    remaining_ships_player = carrier + battleship + cruiser + submarine + destroyer;
    remaining_ships_enemy = remaining_ships_player;
}

void print_board_states() {
    // Print labels
    int padding = ((board_size*2 + 3) - strlen(label_your_board)) / 2;
    printf("  %*c", padding, ' ');
    printf(label_your_board);
    printf("%*c", padding, ' ');

    printf("  ");

    padding = ((board_size*2 + 3) - strlen(label_enemy_board)) / 2;
    printf("%*c", padding, ' ');
    printf(label_enemy_board);
    printf("%*c", padding, ' ');

    printf("\n");

    // Print positional letters
    printf("    ");
    char letter[2] = "a";
    for (int i = 0; i < board_size; i++) {
        printf("%s ", letter);
        (*letter)++;
    }

    printf("     ");
    *letter = 'a';
    for (int i = 0; i < board_size; i++) {
        printf("%s ", letter);
        (*letter)++;
    }

    printf("\n");

    // Draw board wall tops
    printf("  ");
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
        printf("%i %s ", y, board_wall_vertical);
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
        printf("%s %i", board_wall_vertical, y);

        printf("\n");
    }

    // Draw board wall bottoms
    printf("  ");
    printf(board_wall_corner);
    for (int i = 0; i < board_size * 2 + 1; i++) printf(board_wall_horizontal);
    printf(board_wall_corner);

    printf("  ");

    printf(board_wall_corner);
    for (int i = 0; i < board_size * 2 + 1; i++) printf(board_wall_horizontal);
    printf(board_wall_corner);

    printf("\n");
}

bool coords_within_board(int x, int y) {
    // Check if the xy coordinates are within the board
    return (x >= 0 && x < board_size &&
            y >= 0 && y < board_size);
}

bool add_ship(int ship_size, int x, int y, bool horizontal) {
    if (!coords_within_board(x, y)) return false;
    
    // Check if the ship fits in the board
    if (horizontal) {
        if (x + ship_size > board_size)
            return false;
    } else {
        if (y + ship_size > board_size)
            return false;
    }

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

bool add_hit(game_board board, int x, int y) {
    if (!coords_within_board(x, y)) return false;

    if (board[x][y] == shot || board[x][y] == hit)
        return false;
    
    if (board[x][y] == ship)
        board[x][y] = hit;
    else 
        board[x][y] = shot;

    return true;
}

bool add_hit_player(int x, int y) {
    if (!add_hit(player_board, x, y))
        return false;
    
    remaining_ships_player--;
    return true;
}

bool add_hit_enemy(int x, int y) {
    if (!add_hit(enemy_board, x, y))
        return false;
    
    remaining_ships_enemy--;
    return true;
}

void clear_stdin() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) { }
}

bool input_position(int* x, int* y) {
    char x1;
    int y1;

    fflush(stdin);
    if (scanf("%c%i", &x1, &y1) != 2)
        return false;

    *x = x1 - 'a';
    *y = y1;

    return true;
}

bool input_position_direction(int* x, int* y, bool* horizontal) {
    if (!input_position(x, y)) return false;

    char dir;
    if (scanf("%c", &dir) != 1)
        return false;
    
    if (dir == 'h')
        *horizontal = true;
    else if (dir == 'v')
        *horizontal = false;
    else
        return false;
    
    clear_stdin();

    return true;
}

bool add_ship_interactive(int ship_type, int* x_out, int* y_out, bool* horizontal) {
    int x, y;
    bool h;

    if (ship_type >= ship_count)
        return false;
    
    printf("Place the %s, size %d cells\n", ship_names[ship_type], ship_sizes[ship_type]);

    while (true) {
        printf("Enter position and direction (no spaces, h - horizontal, v - vertical, example: a5v):\n");

        if (!input_position_direction(&x, &y, &h)) {
            printf("Invalid input. Press enter to try again.\n");
            continue;
        }

        if (!add_ship(ship_sizes[ship_type], x, y, h)) {
            printf("Cannot place ship there.\n");
            scanf("");
            continue;
        }

        break;
    }

    *x_out = x;
    *y_out = y;
    *horizontal = h;

    return true;
}

bool add_hit_interactive(int* x_out, int* y_out) {
    int x, y;

    printf("Fire at the enemy!\n");
    while (true) {
        printf("Enter position (no spaces, example: a5):\n");

        if (!input_position(&x, &y)) {
            printf("Invalid input. \n");
            scanf("");
            continue;
        }

        if (!add_hit_enemy(x, y)) {
            printf("Cannot shoot there. \n");
            scanf("");
            continue;
        }

        break;
    }

    *x_out = x;
    *y_out = y;

    return true;
}

int main() {
    init();
    add_ship(carrier, 6, 0, false);
    add_ship(battleship, 1, 0, true);
    add_hit(player_board, 0, 0);
    add_hit(player_board, 3, 0);
    add_hit(enemy_board, 5, 5);

    print_board_states();

    int x, y;
    bool h;
    add_ship_interactive(Carrier, &x, &y, &h);

    print_board_states();

    add_hit_interactive(&x, &y);

    print_board_states();

    return 0;
}
