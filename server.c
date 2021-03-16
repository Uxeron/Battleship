#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <stdbool.h>
#include "game.h"
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

int main(int argc, char const *argv[]) 
{ 
    int valread;
    char buffer_in[64] = {0};
    char buffer_out[64] = {0};

    int x, y;
    
    //valread = read( client_socket , buffer, 1024);
    //printf("%s\n",buffer );
    //send(client_socket , hello , strlen(hello) , 0 );
    //printf("Hello message sent\n");

    printf("Waiting for connection...\n");
    create_connection();
    printf("Connection successful, starting game!\n");

    game_init();

    print_board_states();
    add_all_ships_interactive();

    send(client_socket, "0", 2, 0);
    printf("Waiting for other player to finish setup...\n");
    valread = read(client_socket, buffer_in, 64);

    if (buffer_in[0] != '0') {
        perror("Error, invalid data received from client.");
        exit(EXIT_FAILURE);
    }

    while (true) {
        while (true) {
            add_hit_interactive(&x, &y);
            sprintf(buffer_out, "%d,%d", x, y);
            send(client_socket, buffer_out, strlen(buffer_out), 0);
            valread = read(client_socket, buffer_in, 64);

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

        while (true) {
            printf("Waiting for other player to fire...\n");
            valread = read(client_socket, buffer_in, 64);
            if (sscanf(buffer_in, "%d,%d", &x, &y) != 2) {
                perror("Error, invalid data received from client.");
                exit(EXIT_FAILURE);
            }

            add_hit_player(x, y);
            print_board_states();
            if (player_board[x][y] == 3) {
                send(client_socket, "1", 2, 0);
                printf("The other player hit your ship!\n");

                if (check_loss()) {
                    printf("You lose! Too bad.\n");
                    exit(EXIT_SUCCESS);
                }

                continue;
            } else {
                send(client_socket, "0", 2, 0);
                printf("The other player missed.\n");
                break;
            }
        }
    }

    return 0;
} 
