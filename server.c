#include <stdio.h>
#include <netinet/in.h> //network constants and funcs
#include <stdlib.h> //EXIT_*, exit
#include <stdbool.h>
#include "game.h"
#include "common.h"
#define PORT 8080

int client_socket;

void create_connection() {
    struct sockaddr_in address;
    int server_fd;
    int opt = 1;
    int addrlen = sizeof(address);
    
    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    
    // Attaching socket to the port 8080
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( PORT );
    
    // Attaching socket to the port 8080
    if (bind(server_fd, (struct sockaddr*) &address, sizeof(address)) < 0)
    {
        perror("Bind failed.");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 0) < 0)
    {
        perror("Error while listening.");
        exit(EXIT_FAILURE);
    }
    if ((client_socket = accept(server_fd, (struct sockaddr*) &address, (socklen_t*) &addrlen)) < 0)
    {
        perror("Error while accepting connection.");
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char const *argv[]) {
    char buffer_in[64] = {0};

    printf("Waiting for connection...\n");
    create_connection();
    printf("Connection successful, starting game!\n");

    game_init();

    print_board_states();
    add_all_ships_interactive();

    send(client_socket, "0", 2, 0);
    printf("Waiting for other player to finish setup...\n");
    read_socket(client_socket, buffer_in);

    if (buffer_in[0] != '0') {
        perror("Error, invalid data received from client.");
        exit(EXIT_FAILURE);
    }

    while (true) {
        process_your_turn(buffer_in, client_socket);

        process_enemy_turn(buffer_in, client_socket);
    }

    return 0;
} 