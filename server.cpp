#include <stdio.h>
#include <arpa/inet.h> //network constants and funcs
#include <sys/socket.h> //send
#include <unistd.h> //read
#include <stdlib.h> //EXIT_*, exit
#include <fcntl.h>
#include <stdbool.h>
#include <vector>
#include <utility>
#include <errno.h>
#include <string>

#define PORT 8080 
#define BUFFER_SIZE 64

int server_fd;
struct sockaddr_in server;
int addrlen = sizeof(server);

char buffer_in[BUFFER_SIZE] = {0};

int waiting_player = 0;
std::string waiting_player_name;
std::vector<std::pair<int, int>> player_pairs;
std::vector<std::pair<bool, bool>> player_readyness_state;

void read_socket(int socket, char* buffer_in) {
    if (read(socket, buffer_in, BUFFER_SIZE) == -1) {
        perror("Error reading socket data.");
        exit(EXIT_FAILURE);
    }
}

void make_socket_non_blocking(int socket) {
    int flags = fcntl(socket, F_GETFL);
    if (flags == -1) {
        perror("could not get flags on listening socket");
        exit(EXIT_FAILURE);
    }

    if (fcntl(socket, F_SETFL, flags | O_NONBLOCK) == -1) {
        perror("could not set listening socket to be non-blocking");
        exit(EXIT_FAILURE);
    }
}

void setup_listening() {
    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    make_socket_non_blocking(server_fd);

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT);
    
    // Attaching socket
    if (bind(server_fd, (struct sockaddr*) &server, sizeof(server)) < 0) {
        perror("Bind failed.");
        exit(EXIT_FAILURE);
    }

    // Listen for connections
    if (listen(server_fd, 10) < 0) {
        perror("Error while listening.");
        exit(EXIT_FAILURE);
    }
}

void accept_new_players() {
    int new_player = 0;

    // Try to accept all new connections
    while (true) {
        new_player = accept(server_fd, (struct sockaddr*) &server, (socklen_t*) &addrlen);
        if (new_player == -1) {
            if (errno == EWOULDBLOCK) {
                break;
            } else {
                perror("Error while accepting connection.");
                exit(EXIT_FAILURE);
            }
        }

        int read_size = 0;

        // Read the new player's name
        if ((read_size = read(new_player, buffer_in, BUFFER_SIZE)) == -1) {
            perror("Error reading socket data.");
            exit(EXIT_FAILURE);
        }
        //read_socket(new_player, buffer_in);
        std::string new_player_name(buffer_in);
        printf("New player '%s' connected. (%i)\n", new_player_name.c_str(), new_player);

        if (waiting_player == 0) {
            waiting_player = new_player;
            waiting_player_name = new_player_name;
            printf("Player added to waiting queue.\n");
        } else {
            std::pair<int, int> new_player_pair(waiting_player, new_player);
            player_pairs.push_back(new_player_pair);
            player_readyness_state.push_back(std::pair<bool, bool>());

            // Notify the currently waiting player about the new player
            send(waiting_player, "1", 2, 0);
            send(waiting_player, new_player_name.c_str(), new_player_name.length() + 1, 0);

            // Notify the new player about the waiting player
            send(new_player, "0", 2, 0);
            send(new_player, waiting_player_name.c_str(), waiting_player_name.length() + 1, 0);

            printf("New player paired up with player waiting in queue.\n");

            waiting_player = 0;
            waiting_player_name = "";
        }

        make_socket_non_blocking(new_player);
    }
}

bool retransmit_player_messages(int player1, int player2, bool* ready_state_1, bool* ready_state_2) {
    int recv_size = 0;

    if ((recv_size = recv(player1, buffer_in, BUFFER_SIZE, 0)) == -1) {
        if (errno != EWOULDBLOCK) {
            perror("Receive failed.");
            exit(EXIT_FAILURE);
        }
    } else {
        if (recv_size == 2 && buffer_in[0] == '9') {
            printf("Player id %i sent a quit message.\n", player1);
            // Send the connection termination message
            send(player2, buffer_in, recv_size, 0);
            return true;
        }

        /*if (recv_size == 2 && buffer_in[0] == '5' && *ready_state_1 == false) {
            *ready_state_1 = true;
            if (*ready_state_2 == false) // other player not ready yet
                return false;
            else
                send(player1, buffer_in, recv_size, 0); // other player is also ready, notify of the game start
        }*/

        send(player2, buffer_in, recv_size, 0);
    }

    return false;
}

void retransmit_all_player_messages() {
    for (int i = 0; i < player_pairs.size(); i++) {
        auto player1 = player_pairs[i].first;
        auto player2 = player_pairs[i].second;

        bool* ready_state_1 = &(player_readyness_state[i].first);
        bool* ready_state_2 = &(player_readyness_state[i].second);

        if (retransmit_player_messages(player1, player2, ready_state_1, ready_state_2)) {
            // Connection terminate message received
            close(player1);
            close(player2);
            player_pairs.erase(player_pairs.begin() + i);
            player_readyness_state.erase(player_readyness_state.begin() + i);
            i--;
        }

        if (retransmit_player_messages(player2, player1, ready_state_2, ready_state_1)) {
            // Connection terminate message received
            close(player1);
            close(player2);
            player_pairs.erase(player_pairs.begin() + i);
            player_readyness_state.erase(player_readyness_state.begin() + i);
            i--;
        }
    }
}

int main(int argc, char const *argv[]) {
    printf("Setting up listening socket...\n");
    setup_listening();
    printf("Setup done, listening for connections.\n");

    while (true) {
        accept_new_players();
        retransmit_all_player_messages();
    }

    return 0;
} 
