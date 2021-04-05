#include <stdio.h>
#include <arpa/inet.h> //network constants and funcs
#include <sys/socket.h> //send
#include <unistd.h> //read
#include <stdlib.h> //EXIT_*, exit
#include <stdbool.h>
#include "game.hpp"
#include "common.hpp"

int client_socket;

void create_connection() {
    struct sockaddr_in server;
    int server_fd;
    int addrlen = sizeof(server);
    
    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT);
    
    // Attaching socket
    if (bind(server_fd, (struct sockaddr*) &server, sizeof(server)) < 0)
    {
        perror("Bind failed.");
        exit(EXIT_FAILURE);
    }

    // Listen for connections
    if (listen(server_fd, 0) < 0)
    {
        perror("Error while listening.");
        exit(EXIT_FAILURE);
    }

    // Accept the first received connection
    if ((client_socket = accept(server_fd, (struct sockaddr*) &server, (socklen_t*) &addrlen)) < 0)
    {
        perror("Error while accepting connection.");
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char const *argv[]) {
    char buffer_in[BUFFER_SIZE] = {0};

    printf("Waiting for connection...\n");
    create_connection();
    printf("Connection successful, starting game!\n");

    Game* game = new Game();

    game->print_board_states();
    game->add_all_ships_interactive();

    send(client_socket, "0", 2, 0);
    printf("Waiting for other player to finish setup...\n");
    read_socket(client_socket, buffer_in);

    if (buffer_in[0] != '0') {
        perror("Error, invalid data received from client.");
        exit(EXIT_FAILURE);
    }

    while (true) {
        process_your_turn(game, buffer_in, client_socket);

        process_enemy_turn(game, buffer_in, client_socket);
    }

    return 0;
} 
