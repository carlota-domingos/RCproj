#include <stddef.h>
#include <cstdio>
#include <arpa/inet.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <random>
#include <unistd.h>
#include <regex>
#include <string>
#include <sstream>
#include <filesystem>
#include "udpTcp.h"

using namespace std;

#define BUFFER_SIZE 128
//--------------------------------------UDP SERVER------------------------------------------------------------------------------------------

// Função para inicializar o socket
int init_socket_server(char* PORT) {
    int fd_udp = socket(AF_INET, SOCK_DGRAM, 0); 
    if (fd_udp == -1) {
        perror("Erro ao criar socket");
        return -1;
    }
    struct addrinfo *infoaddr, hints;
    memset(&hints, 0, sizeof(hints));

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;
    
    int errcode_udp = getaddrinfo(NULL,PORT , &hints, &infoaddr);

    if (errcode_udp != 0) {
        perror("Erro ao resolver endereço");
        close(fd_udp);
        return -1;
    }
    // bind
    int n = bind(fd_udp, infoaddr->ai_addr, infoaddr->ai_addrlen);
    if (n == -1) {
        perror("Erro ao vincular o socket");
        return -1;
    }
    return fd_udp; 
}

// Função para receber mensagem
ssize_t receive_message_server(int fd_udp, char* buffer, size_t buffer_size, struct sockaddr_in& addr, socklen_t& addrlen_udp) {
    return recvfrom(fd_udp, buffer, buffer_size, 0, (struct sockaddr*)&addr, &addrlen_udp);
}

// Função para enviar mensagem
int send_message_server(int fd_udp, const char* buffer, size_t length, struct sockaddr_in& addr, socklen_t addrlen_udp) {
    ssize_t n = sendto(fd_udp, buffer, length, 0, (struct sockaddr*)&addr, addrlen_udp);
    if (n == -1) {
        perror("Erro ao enviar mensagem");
        return -1;
    }
    return 0; 
}

//--------------------------------------UDP PLAYER------------------------------------------------------------------------------------------------------

// Função para inicializar o socket UDP
int init_socket_player(char *hostname, struct addrinfo *&infoaddr,char* PORT) {
    int fd_udp = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd_udp < 0) {
        perror("Erro ao criar socket");
        return -1;
    }

    addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    int errcode_udp = getaddrinfo(hostname, PORT, &hints, &infoaddr);

    if (errcode_udp != 0)   {
        perror("Erro ao resolver endereço");
        close(fd_udp);
        return -1;
    }

    return fd_udp; 
}

// Função para enviar mensagem
int send_socket_udp_player(int fd_udp, const char *message, struct addrinfo *infoaddr){
    ssize_t n = sendto(fd_udp, message, strlen(message), 0, infoaddr->ai_addr, infoaddr->ai_addrlen);
    if (n == -1)    {
        perror("Erro ao enviar mensagem"); 
        return -1;
    }
    return 0; 
}

// Função para receber mensagem
int receive_socket_udp_player(int fd_udp, char *buffer, size_t buffer_size){
    sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);

    ssize_t n = recvfrom(fd_udp, buffer, buffer_size, 0, (struct sockaddr *)&addr, &addrlen);
    if (n < 0) {
        perror("recvfrom");
        return -1;
    }
    buffer[n] = '\0'; 
    return n;        
}


//--------------------------------------TCP SERVER------------------------------------------------------------------------------------------------------

// Função para inicializar o servidor TCP
int init_tcp_server(char *port) {
    int fd;
    struct addrinfo hints, *res;
    int errcode;

    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1) {
        perror("socket");
        exit(1);
    }

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    errcode = getaddrinfo(NULL, port, &hints, &res);

    if (errcode != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(errcode));
        exit(1);
    }

    if (bind(fd, res->ai_addr, res->ai_addrlen) == -1) {
        perror("bind");
        freeaddrinfo(res);
        exit(1);
    }

    if (listen(fd, 5) == -1) {
        perror("listen");
        freeaddrinfo(res);
        exit(1);
    }

    return fd;
}

// Função para aceitar a conexão TCP
int accept_connection_tcp_server(int server_fd, struct sockaddr_in *addr, socklen_t *addrlen) {
    int client_fd = accept(server_fd, (struct sockaddr *)addr, addrlen);
    if (client_fd == -1) {
        perror("accept");
        exit(1);
    }
    return client_fd;
}

// Função para ler a mensagem
ssize_t read_message_tcp_server(int client_fd, char *buffer, size_t size) {
    ssize_t n = read(client_fd, buffer, size);
    if (n == -1) {
        perror("read");
        exit(1);
    }
    return n;
}

// Função para enviar a mensagem
int send_message_tcp_server(int client_fd, const char *message, ssize_t size) {
    ssize_t n = write(client_fd, message, size);
    if (n == -1) {
        perror("write");
        return -1;
    }
    return 0;
}

//--------------------------------------TCP PLAYER------------------------------------------------------------------------------------------------------

// Função para inicializar o socket TCP do jogador
int init_tcp_player(char *hostname, struct addrinfo *&infoaddr, char* PORT) {
    int fd;
    struct addrinfo hints;
    int errcode;

    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1) {
        perror("socket");
        exit(1);
    }

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    errcode = getaddrinfo(hostname, PORT, &hints, &infoaddr);

    if (errcode != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(errcode));
        close(fd);
        exit(1);
    }

    if (connect(fd, infoaddr->ai_addr, infoaddr->ai_addrlen) == -1) {
        cout << PORT << endl;
        perror("connect");
        freeaddrinfo(infoaddr);
        close(fd);
        exit(1);
    }
    return fd;
}

// Função para enviar a mensagem
int send_tcp_player(int fd, const char *message) {
    int size = strlen(message);
    ssize_t n;
    while(size > 0) {
        n = write(fd, message, size);
        if (n < 0) {
            printf("Erro ao enviar mensagem no loop tcp\n");
            return -1;
        }
        size -= n;
    }
    return 0;
}

// Função para receber mensagem
ssize_t receive_tcp_player(int fd, char *buffer, size_t size) {
    size_t i = 0;
    ssize_t bytes_read;

    while (i < size - 1) { 
        bytes_read = read(fd, buffer + i, 1); 
        if (bytes_read == -1) {
            perror("Error while reading from socket");
            return -1;
        } else if (bytes_read == 0) 
            break;
        i++;
    }
    buffer[i] = '\0';
    return i; 
}
