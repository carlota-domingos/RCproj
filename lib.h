#ifndef LIB_H_INCLUDED
#define LIB_H_INCLUDED
#include "lib.cpp"
#include <stddef.h>
#include <netdb.h>
#include <cstdio>
#include <arpa/inet.h>
#include <iostream>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <fstream>
#include <sys/stat.h>
#include <unistd.h>
#include <filesystem>


using namespace std;



void create_directories();
void create_file(const string& directory, const string& filename);

int get_msg(string &msg);
int case_(string &buffer);
int code_val(const string& code);

int receive_socket_udp_player(int fd_udp, char *buffer, size_t buffer_size);
int send_socket_udp_player(int fd_udp, const char *message, struct addrinfo *infoaddr);
int init_socket_player(const char *hostname, struct addrinfo *&infoaddr);

int send_message_server(int fd_udp, const char* buffer, size_t length, struct sockaddr_in& addr, socklen_t addrlen_udp) ;
ssize_t receive_message_server(int fd_udp, char* buffer, size_t buffer_size, struct sockaddr_in& addr, socklen_t& addrlen_udp) ;
int bind_socket_server(int fd_udp, struct addrinfo* infoaddr) ;

int init_tcp_player(const char *hostname, const char *port);
int send_tcp_player(int fd, const char *message);
ssize_t receive_tcp_player(int fd, char *buffer, size_t size);

int init_tcp_server(const char *port);
int accept_connection_tcp_server(int server_fd, struct sockaddr_in *addr, socklen_t *addrlen);
ssize_t read_message_tcp_server(int client_fd, char *buffer, size_t size);
void echo_message_tcp_player(int client_fd, const char *message, ssize_t size);


#endif
