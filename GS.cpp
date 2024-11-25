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
#include <filesystem>


#define PORT "58001"
#define BUFFER_SIZE 128
#define NUM_COLORS 4

int case_(const char* buffer_received) {
    string buffer(buffer_received); // Converte o buffer recebido em um std::string
    string PLID; // Usaremos std::string para o PLID

    // Caso SCORES
    if (buffer.compare("SSB") == 0) {
        
    }
    //Caso GAMES
    else if (buffer.substr(0, 4).compare("STR ") == 0 && buffer.size() == 10) {
        PLID = buffer.substr(4, 6); 
    }
    //caso QUIT
    else if (buffer.substr(0, 4).compare("QUT ") == 0 && buffer.size() == 10) {
        PLID = buffer.substr(4, 6);
        // Quit do jogador com o PLID
    }
    // caso DEBUG 
    else if (buffer.substr(0, 4).compare("DBG ") == 0 && buffer.size() == 14) {
        PLID = buffer.substr(4, 6);
        int tempo_max = stoi(buffer.substr(11, 3));
    }
    // TRY CASE
    else if (buffer.substr(0, 4).compare("TRY ") == 0 && buffer.size() == 10) {
        PLID = buffer.substr(4, 6);
        
    }
    // START NEW GAME CASE
    else if (buffer.substr(0, 4).compare("SNG ") == 0 && buffer.size() == 14) {
        PLID = buffer.substr(4, 6);
        int tempo_max = stoi(buffer.substr(11, 3)); 
        
    }
    else {
        perror("Algo correu mal case_ do server");
        return -1; // Indica erro
    }
    return 0; // Sucesso
}



// Função principal
int main() {
    
    struct addrinfo* infoaddr = nullptr;
    struct sockaddr_in addr{};
    char buffer[BUFFER_SIZE];
    socklen_t addrlen_udp = sizeof(addr);

    create_directories();
    //create_file("SCORES", "scoreboard.txt");

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
        
        case_(buffer);

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
