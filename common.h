#pragma once

#define PORT 8080 
#define BUFFER_SIZE 64

void read_socket(int socket, char* buffer_in);
void process_your_turn(char* buffer_in, int socket);
void process_enemy_turn(char* buffer_in, int socket);