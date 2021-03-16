#include <stdio.h>
#include <sys/socket.h> //send
#include <stdlib.h> //EXIT_*, exit
#include <unistd.h> //read
#include <string.h> //strlen
#include <stdbool.h>
#include "game.h"

void read_socket(int socket, char* buffer_in) {
    if (read(socket, buffer_in, sizeof(buffer_in) / sizeof(*buffer_in)) == -1) {
        perror("Error reading socket data.");
        exit(EXIT_FAILURE);
    }
}

void process_your_turn(char* buffer_in, int socket) {
    char buffer_out[64] = {0};
    int x, y;

    while (true) {
        add_hit_interactive(&x, &y);
        sprintf(buffer_out, "%d,%d", x, y);
        send(socket, buffer_out, strlen(buffer_out), 0);
        read_socket(socket, buffer_in);

        if (buffer_in[0] == '1') {
            add_hit_enemy(x, y, 3);
            print_board_states();
            printf("That's a hit!\n");

            if (check_victory()) {
                printf("You win! Congratulations!\n");
                exit(EXIT_SUCCESS);
            }

            continue;
        } else {
            add_hit_enemy(x, y, 2);
            print_board_states();
            printf("That's a miss.\n");
            break;
        }
    }
}

void process_enemy_turn(char* buffer_in, int socket) {
    int x, y;

    while (true) {
        printf("Waiting for other player to fire...\n");
        read_socket(socket, buffer_in);
        if (sscanf(buffer_in, "%d,%d", &x, &y) != 2) {
            perror("Error, invalid data received from server.");
            exit(EXIT_FAILURE);
        }

        add_hit_player(x, y);
        print_board_states();
        if (player_board[x][y] == 3) {
            send(socket, "1", 2, 0);
            printf("The other player hit your ship!\n");

            if (check_loss()) {
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