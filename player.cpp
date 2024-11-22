#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <iostream>
#include <cstdio>
#include <arpa/inet.h>
#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <unistd.h>

#define PORT "58001"
#define BUFFER_SIZE 128


// Função para inicializar o socket
int init_socket(const char *hostname, struct addrinfo *&infoaddr){
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

// Função para enviar mensagem???????????????????????????????
int send_socket_udp(int fd_udp, const char *message, struct addrinfo *infoaddr)
{
    ssize_t n = sendto(fd_udp, message, strlen(message), 0, infoaddr->ai_addr, infoaddr->ai_addrlen);
    if (n == -1)    {
        perror("Erro ao enviar mensagem");
        return -1;
    }
    return 0; 
}

// Função para receber mensagem
int receive_socket_udp(int fd_udp, char *buffer, size_t buffer_size)
{
    sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);

    ssize_t n = recvfrom(fd_udp, buffer, buffer_size, 0, (struct sockaddr *)&addr, &addrlen);
    if (n == -1)    {
        perror("Erro ao receber mensagem");
        return -1;
    }

    buffer[n] = '\0'; // Certifica-se de que o buffer termina com '\0' (para strings)
    return n;         // Retorna o número de bytes recebidos
}

//funcao para ler do terminal
int get_msg(char *msgbuffer) {
    char c;
    int i = 0;
    while ((c = getchar())!= EOF || c != '\n' || i < 128) {
        if(c =='\n'){
            break;
        }
        msgbuffer[i] = c;
        i++;
    }
    msgbuffer[i] = '\0';    
    return 0;
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


int main() {
    struct addrinfo *infoaddr = nullptr; // Ponteiro para guardar informações do endereço
    char buffer[BUFFER_SIZE];
    //char msgbuffer[BUFFER_SIZE];
    char sendmsg[BUFFER_SIZE];  

    // Inicializa o socket
    int fd_udp = init_socket("localhost", infoaddr);
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
        if (send_socket_udp(fd_udp, sendmsg , infoaddr) < 0)   {
            freeaddrinfo(infoaddr);
            close(fd_udp);
            return 1;
        }

        // Recebe mensagem
        int n = receive_socket_udp(fd_udp, buffer, BUFFER_SIZE);
        if (n < 0)  {
            freeaddrinfo(infoaddr);
            close(fd_udp);
            return 1;
        }
        // Imprime a mensagem recebida
        write(1, "echo: ", 6);
        write(1, buffer, n);
        write(1, "\n", 1);

    }   
    
    

    // Limpeza
    freeaddrinfo(infoaddr);
    close(fd_udp);
    return 0;
}
