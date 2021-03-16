#include <stdio.h>
#include <arpa/inet.h> //network constants and funcs
#include <stdlib.h> //EXIT_*, exit
#include <stdbool.h>
#include "game.h"
#include "common.h"
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

int main(int argc, char const *argv[]) {
	char buffer_in[64] = {0};

	printf("Attempting connection...\n");
    create_connection();
    printf("Connection successful, starting game!\n");

    game_init();

    print_board_states();
    add_all_ships_interactive();

    send(server_socket, "0", 2, 0);
    printf("Waiting for other player to finish setup...\n");
    read_socket(server_socket, buffer_in);

    if (buffer_in[0] != '0') {
        perror("Error, invalid data received from server.");
        exit(EXIT_FAILURE);
    }

    while (true) {
        process_enemy_turn(buffer_in, server_socket);

        process_your_turn(buffer_in, server_socket);
    }
}
