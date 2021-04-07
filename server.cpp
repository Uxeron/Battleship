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
#include <string.h>
#include <time.h>
#include <signal.h>

#define PORT 8080 
#define BUFFER_SIZE 64

// Used for sending the signal
#define SERVER_DISCONNECT_S "8"
#define CLIENT_DISCONNECT_S "9"

// Used for receiving the signal
#define SERVER_DISCONNECT_R '8'
#define CLIENT_DISCONNECT_R '9'

int server_fd;
struct sockaddr_in server;
int addrlen = sizeof(server);

char buffer_in[BUFFER_SIZE] = {0};

int waiting_player = 0;
std::string waiting_player_name;
std::vector<std::pair<int, int>> player_pairs;

struct sigaction old_action;

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
    // Create listening socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    make_socket_non_blocking(server_fd);

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT);
    
    // Attach socket
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

        // Read the new player's name
        if (recv(new_player, buffer_in, BUFFER_SIZE, 0) == -1) {
            perror("Error reading socket data.");
            exit(EXIT_FAILURE);
        }

        std::string new_player_name(buffer_in);
        printf("New player '%s' connected. (%i)\n", new_player_name.c_str(), new_player);

        if (waiting_player == 0) {
            waiting_player = new_player;
            waiting_player_name = new_player_name;
            printf("Player added to waiting queue.\n");
        } else {
            std::pair<int, int> new_player_pair(waiting_player, new_player);
            player_pairs.push_back(new_player_pair);

            // Notify the currently waiting player about the new player
            nanosleep((const struct timespec[]){{0, 10000000L}}, NULL);
            send(waiting_player, "1", 2, 0);
            send(waiting_player, new_player_name.c_str(), new_player_name.length() + 1, 0);

            // Notify the new player about the waiting player
            nanosleep((const struct timespec[]){{0, 10000000L}}, NULL);
            send(new_player, "0", 2, 0);
            send(new_player, waiting_player_name.c_str(), waiting_player_name.length() + 1, 0);

            printf("New player paired up with player waiting in queue.\n");

            waiting_player = 0;
            waiting_player_name = "";
        }

        make_socket_non_blocking(new_player);
    }
}

bool retransmit_player_messages(int player1, int player2) {
    int recv_size = 0;

    if ((recv_size = recv(player1, buffer_in, BUFFER_SIZE, 0)) == -1) {
        if (errno != EWOULDBLOCK) {
            perror("Receive failed.");
            exit(EXIT_FAILURE);
        }
    } else {
        if (recv_size == 2 && buffer_in[0] == CLIENT_DISCONNECT_R) {
            printf("Player id %i sent a quit message.\n", player1);
            // Send the connection termination message
            send(player2, buffer_in, recv_size, 0);
            return true;
        }

        send(player2, buffer_in, recv_size, 0);
    }

    return false;
}

void retransmit_all_player_messages() {
    if (waiting_player != 0) {
        int recv_size = 0;
        if ((recv_size = recv(waiting_player, buffer_in, BUFFER_SIZE, 0)) == -1) {
            if (errno != EWOULDBLOCK) {
                perror("Receive failed.");
                exit(EXIT_FAILURE);
            }
        } else {
            if (recv_size == 2 && buffer_in[0] == CLIENT_DISCONNECT_R) {
                printf("Waiting player id %i sent a quit message.\n", waiting_player);
                waiting_player = 0;
                waiting_player_name = "";
            }
        }
    }

    for (int i = 0; i < player_pairs.size(); i++) {
        int player1 = player_pairs[i].first;
        int player2 = player_pairs[i].second;

        if (retransmit_player_messages(player1, player2)) {
            // Connection terminate message received
            close(player1);
            close(player2);
            player_pairs.erase(player_pairs.begin() + i);
            i--;
            continue;
        }

        if (retransmit_player_messages(player2, player1)) {
            // Connection terminate message received
            close(player1);
            close(player2);
            player_pairs.erase(player_pairs.begin() + i);
            i--;
            continue;
        }
    }
}

void sigint_handler(int sig_no)
{
    printf("CTRL-C pressed\n");

    if (waiting_player != 0)
        send(waiting_player, SERVER_DISCONNECT_S, 2, 0);

    for (int i = 0; i < player_pairs.size(); i++) {
        send(player_pairs[i].first, SERVER_DISCONNECT_S, 2, 0);
        send(player_pairs[i].second, SERVER_DISCONNECT_S, 2, 0);
    }

    sigaction(SIGINT, &old_action, NULL);
    kill(0, SIGINT);
}

int main(int argc, char const *argv[]) {
    // Setup ctrl+c capture
    struct sigaction action;
    memset(&action, 0, sizeof(action));
    action.sa_handler = &sigint_handler;
    sigaction(SIGINT, &action, &old_action);

    printf("Setting up listening socket...\n");
    setup_listening();
    printf("Setup done, listening for connections.\n");

    while (true) {
        accept_new_players();
        retransmit_all_player_messages();
    }

    return 0;
} 
