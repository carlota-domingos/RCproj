#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <iostream>
#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <unistd.h>
#define PORT "58001"
#define BUFFER_SIZE 128
#define NUM_COLORS 4



//funcao de criar diretoria
void create_directories() {
    // Criando o diretório scoreboard
    if (mkdir("SCORES", 0777) == -1) {
        perror("Erro ao criar diretório SCORES");
    } 

    // Criando o diretório show_trials
    if (mkdir("GAMES", 0777) == -1) {
        perror("Erro ao criar diretório GAMES");
    }
}

// Função para inicializar o socket
int init_socket(struct addrinfo*& infoaddr) {
    int fd_udp = socket(AF_INET, SOCK_DGRAM, 0); 
    if (fd_udp == -1) {
        perror("Erro ao criar socket");
        return -1;
    }

    addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    int errcode_udp = getaddrinfo(NULL, PORT, &hints, &infoaddr);
    if (errcode_udp != 0) {
        perror("Erro ao resolver endereço");
        close(fd_udp);
        return -1;
    }

    return fd_udp; 
}

// Função para vincular o socket ao endereço
int bind_socket(int fd_udp, struct addrinfo* infoaddr) {
    int n = bind(fd_udp, infoaddr->ai_addr, infoaddr->ai_addrlen);
    if (n == -1) {
        perror("Erro ao vincular o socket");
        return -1;
    }
    return 0; // Sucesso
}

// Função para receber mensagem
ssize_t receive_message(int fd_udp, char* buffer, size_t buffer_size, struct sockaddr_in& addr, socklen_t& addrlen_udp) {
    return recvfrom(fd_udp, buffer, buffer_size, 0, (struct sockaddr*)&addr, &addrlen_udp);
}

// Função para enviar mensagem
int send_message(int fd_udp, const char* buffer, size_t length, struct sockaddr_in& addr, socklen_t addrlen_udp) {
    ssize_t n = sendto(fd_udp, buffer, length, 0, (struct sockaddr*)&addr, addrlen_udp);
    if (n == -1) {
        perror("Erro ao enviar mensagem");
        return -1;
    }
    return 0; // Sucesso
}

//gera o codigo de coderes para o jogo
void generate_random_colors(char *result) {
    char colors[] = {'R', 'G', 'B', 'Y', 'O', 'P'};
    size_t num_available_colors = sizeof(colors) / sizeof(colors[0]);

    for (int i = 0; i < NUM_COLORS; i++) {
        int random_index = rand() % num_available_colors; // Escolhe um índice aleatório
        result[i] = colors[random_index];                // Adiciona a cor à sequência
    }
    result[NUM_COLORS] = '\0'; // Adiciona o terminador nulo para tornar a string válida
}


/*void switch_case(const char *buffer) {
    int i=0;
    char c;
    size_t len = strlen(buffer);          
    switch (c = buffer[i])
    {
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
}*/


// Função principal
int main() {
    struct addrinfo* infoaddr = nullptr;
    struct sockaddr_in addr{};
    char buffer[BUFFER_SIZE];
    socklen_t addrlen_udp = sizeof(addr);
    create_directories();

    // Inicializa o socket
    int fd_udp = init_socket(infoaddr);
    if (fd_udp < 0) {
        return 1;
    }

    // Vincula o socket ao endereço
    if (bind_socket(fd_udp, infoaddr) < 0) {
        freeaddrinfo(infoaddr);
        close(fd_udp);
        return 1;
    }

    //colocar aqui especial ler a 1a msgm e iniciar o jogo

    
    // Loop de recepção e envio de mensagens durante o jogo apenas
    while (1) {
        ssize_t n = receive_message(fd_udp, buffer, BUFFER_SIZE, addr, addrlen_udp);
        if (n == -1) {
            freeaddrinfo(infoaddr);
            close(fd_udp);
            return 1; 
        }

        write(1, "received: ", 10);
        write(1, buffer, n);
        write(1, "\n", 1);
        

        // Envia a mensagem de volta para o endereço de onde foi recebida
        if (send_message(fd_udp, "tudo tudo ok", 12, addr, addrlen_udp) < 0) {
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
