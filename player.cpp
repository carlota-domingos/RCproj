#include namespace std
#include <unistd.h>
#include <stdlib.h>
#include <stddef.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <cstdio>
#include <arpa/inet.h>
#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include "lib.h"

#define PORT "58001"
#define BUFFER_SIZE 128


int code_val(const std::string& code) {
    std::regex pattern("^([RGBYOP]) ([RGBYOP]) ([RGBYOP]) ([RGBYOP])$");
    
    return std::regex_match(code, pattern);
}



int main() {
    struct addrinfo *infoaddr = nullptr; // Ponteiro para guardar informações do endereço
    char buffer[BUFFER_SIZE];
    //char msgbuffer[BUFFER_SIZE];
    char sendmsg[BUFFER_SIZE];  

    // Inicializa o socket
    int fd_udp = init_socket_player("localhost", infoaddr);
    if (fd_udp < 0) {
        return 1;
    }
   
   //loop para durante o jogo                                                     
    while (1) {
        if (get_msg(sendmsg) != 0)  {
            perror("Erro ao ler a mensagem");
            return -1;
        } 

        printf("%s", "a");
        // Envia mensagem
        if (send_socket_udp_player(fd_udp, sendmsg , infoaddr) < 0)   {
            freeaddrinfo(infoaddr);
            close(fd_udp);
            return 1;
        }

        // Recebe mensagem
        int n = receive_socket_udp_player(fd_udp, buffer, BUFFER_SIZE);
        if (n < 0)  {
            freeaddrinfo(infoaddr);
            close(fd_udp);
            return 1;
        }
        // Imprime a mensagem recebida
        write(1, "echo: ", 6);
        write(1, buffer, n);
        write(1, "\n", 1);

        memset(buffer, 0, n);


    }   
    // Limpeza
    freeaddrinfo(infoaddr);
    close(fd_udp);
    return 0;
}
