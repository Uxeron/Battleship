#pragma once

void read_socket(int socket, char* buffer_in);
void process_your_turn(char* buffer_in, int socket);
void process_enemy_turn(char* buffer_in, int socket);