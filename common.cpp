#include <stdio.h>
#include <sys/socket.h> //send
#include <unistd.h> //read
#include <stdlib.h> //EXIT_*, exit
#include <string.h> //strlen
#include <stdbool.h>
#include "game.hpp"
#include "common.hpp"

void read_socket(int socket, char* buffer_in) {
    if (read(socket, buffer_in, BUFFER_SIZE) == -1) {
        perror("Error reading socket data.");
        exit(EXIT_FAILURE);
    }
}

void process_your_turn(Game* game, char* buffer_in, int socket) {
    char buffer_out[BUFFER_SIZE] = {0};
    int x, y;

    while (true) {
        game->add_hit_interactive(&x, &y);
        sprintf(buffer_out, "%d,%d", x, y);
        send(socket, buffer_out, strlen(buffer_out), 0);
        read_socket(socket, buffer_in);

        if (buffer_in[0] == '1') {
            game->add_hit_enemy(x, y, 3);
            game->print_board_states();
            printf("That's a hit!\n");

            if (game->check_victory()) {
                printf("You win! Congratulations!\n");
                exit(EXIT_SUCCESS);
            }

            continue;
        } else {
            game->add_hit_enemy(x, y, 2);
            game->print_board_states();
            printf("That's a miss.\n");
            break;
        }
    }
}

void process_enemy_turn(Game* game, char* buffer_in, int socket) {
    int x, y;

    while (true) {
        printf("Waiting for other player to fire...\n");
        read_socket(socket, buffer_in);
        if (sscanf(buffer_in, "%d,%d", &x, &y) != 2) {
            perror("Error, invalid data received from server.");
            exit(EXIT_FAILURE);
        }

        game->add_hit_player(x, y);
        game->print_board_states();
        if (game->player_board[x][y] == 3) {
            send(socket, "1", 2, 0);
            printf("The other player hit your ship!\n");

            if (game->check_loss()) {
                printf("You lose! Too bad.\n");
                exit(EXIT_SUCCESS);
            }

            continue;
        } else {
            send(socket, "0", 2, 0);
            printf("The other player missed.\n");
            break;
        }
    }
}