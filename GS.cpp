#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <unistd.h>
#include "lib.h"

#define PORT "58001"
#define BUFFER_SIZE 128
#define NUM_COLORS 4

/*
void case_( char *buffe<r) {
    size_t len = strlen(buffer);          
    switch (c = buffer[i]) {
    case //c seja um numero e len_buff < 10 (caso normal) :
        // code 
        break;
    case //c seja uma letra espaco letra :
        // code 
        break;
    case //c0 seja s e c1 seja t ou palavra show_trials :
        // code 
        break;
    case //c0 seja s e c1 seja b ou palavra scoreboard:
        // code 
        break;
    case //quit:
        // code 
        break;
    case //exit:
        // code 
        break;
    case //c seja um numero e len_buff < 10 (caso normal):
        // code 
        break;

    default:
        break;
    }
}
*/

// Função principal
int main() {
    struct addrinfo* infoaddr = nullptr;
    struct sockaddr_in addr{};
    char buffer[BUFFER_SIZE];
    socklen_t addrlen_udp = sizeof(addr);
    create_directories();

    // Inicializa o socket
    int fd_udp = init_socket_server(infoaddr);
    if (fd_udp < 0) {
        return 1;
    }

    // Vincula o socket ao endereço
    if (bind_socket_server(fd_udp, infoaddr) < 0) {
        freeaddrinfo(infoaddr);
        close(fd_udp);
        return 1;
    }


    
    // Loop de recepção e envio de mensagens durante o jogo apenas
    while (1) {
        ssize_t n = receive_message_server(fd_udp, buffer, BUFFER_SIZE, addr, addrlen_udp);
        if (n == -1) {
            freeaddrinfo(infoaddr);
            close(fd_udp);
            return 1; 
        }

        write(1, "received: ", 10);
        write(1, buffer, n);
        write(1, "\n", 1);
        
        // case_(buffer);

        // Envia a mensagem de volta para o endereço de onde foi recebida
        if (send_message_server(fd_udp, "tudo tudo ok", 12, addr, addrlen_udp) < 0) {
            freeaddrinfo(infoaddr);
            close(fd_udp);
            return 1; // Erro ao enviar mensagem
        }
    }

    // Liberação de recursos
    freeaddrinfo(infoaddr);
    close(fd_udp);
    return 0;
}
