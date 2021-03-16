#include <stdio.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <stdlib.h>
#include <unistd.h> 
#include <string.h> 
#include <stdbool.h>
#include "game.h"
#define PORT 8080 

int server_socket;

void create_connection() {
	struct sockaddr_in serv_addr;
	
	if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("Socket creation error.");
		exit(EXIT_FAILURE);
	}

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);
	
	// Convert IPv4 and IPv6 addresses from text to binary form
	if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0)
	{
		perror("Invalid address/ Address not supported.");
		exit(EXIT_FAILURE);
	}

	if (connect(server_socket, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0)
	{
		perror("Connection Failed.");
		exit(EXIT_FAILURE);
	}
}

int main(int argc, char const *argv[])
{
    int valread;
	char buffer_in[64] = {0};
    char buffer_out[64] = {0};

    int x, y;

	printf("Attempting connection...\n");
    create_connection();
    printf("Connection successful, starting game!\n");

    game_init();

    print_board_states();
    add_all_ships_interactive();

    send(server_socket, "0", 2, 0);
    printf("Waiting for other player to finish setup...\n");
    valread = read(server_socket, buffer_in, 64);

    if (buffer_in[0] != '0') {
        perror("Error, invalid data received from server.");
        exit(EXIT_FAILURE);
    }

    while (true) {
        while (true) {
            printf("Waiting for other player to fire...\n");
            valread = read(server_socket, buffer_in, 64);
            if (sscanf(buffer_in, "%d,%d", &x, &y) != 2) {
                perror("Error, invalid data received from server.");
                exit(EXIT_FAILURE);
            }

            add_hit_player(x, y);
            print_board_states();
            if (player_board[x][y] == 3) {
                send(server_socket, "1", 2, 0);
                printf("The other player hit your ship!\n");

                if (check_loss()) {
                    printf("You lose! Too bad.\n");
                    exit(EXIT_SUCCESS);
                }

                continue;
            } else {
                send(server_socket, "0", 2, 0);
                printf("The other player missed.\n");
                break;
            }
        }

        while (true) {
            add_hit_interactive(&x, &y);
            sprintf(buffer_out, "%d,%d", x, y);
            send(server_socket, buffer_out, strlen(buffer_out), 0);
            valread = read(server_socket, buffer_in, 64);

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
                printf("That's a miss\n");
                break;
            }
        }
    }
}
