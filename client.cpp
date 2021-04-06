#include <stdio.h>
#include <arpa/inet.h> //network constants and funcs
#include <sys/socket.h> //send
#include <unistd.h> //read
#include <stdlib.h> //EXIT_*, exit
#include <stdbool.h>
#include <string>
#include "game.hpp"
#include "common.hpp"

int server_socket;

void create_connection(std::string name) {
	struct sockaddr_in server;
	
	if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("Socket creation error.");
		exit(EXIT_FAILURE);
	}

	server.sin_family = AF_INET;
	server.sin_port = htons(PORT);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");

	if (connect(server_socket, (struct sockaddr*) &server, sizeof(server)) < 0)
	{
		perror("Connection Failed.");
		exit(EXIT_FAILURE);
	}

    // Tell the server this player's name
    send(server_socket, name.c_str(), name.size() + 1, 0);
}

int main(int argc, char const *argv[]) {
	char buffer_in[BUFFER_SIZE] = {0};

    char name[100];

    printf("Enter your name: ");
    scanf("%s", &name);

	printf("Attempting connection...\n");
    create_connection(std::string(name));

    printf("Connection successful, waiting for pairing...\n");

    // Determine who starts the game
    read_socket(server_socket, buffer_in);
    if (buffer_in[0] != '0' && buffer_in[0] != '1') {
            perror("Error, invalid data received from server.");
            exit(EXIT_FAILURE);
    }
    bool starting = buffer_in[0] - '0'; // If received '0' - the other player starts, if '1' - you start

    // Get opponent's name
    read_socket(server_socket, buffer_in);
    std::string p2_name(buffer_in);

    printf("Paired up with player %s, starting game!", p2_name.c_str());

    Game game;

    game.clear_stdin();

    game.print_board_states();
    game.add_all_ships_interactive();

    send(server_socket, "5", 2, 0);
    printf("Waiting for other player to finish setup...\n");
    read_socket(server_socket, buffer_in);

    if (buffer_in[0] != '5') {
        perror("Error, invalid data received from server.");
        exit(EXIT_FAILURE);
    }

    if (starting)
        process_your_turn(&game, buffer_in, server_socket);

    while (true) {
        process_enemy_turn(&game, buffer_in, server_socket);

        process_your_turn(&game, buffer_in, server_socket);
    }
}
