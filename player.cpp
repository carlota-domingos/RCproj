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

using namespace std;


#define PORT "58001"
#define BUFFER_SIZE 128
// class game_player{
//     public:
//         string plid;
//         int nT;
//         void _init_(string id){
//             plid= id;
//             nT= 0;
//         }
//         void next_try(){
//             nT++;
//         }
//         bool same_try(int server_try){
//             return nT==server_try;
//         }
// }

// game_player curr_game;

// int add_args(string &msg, int code){
//     switch(code){
//         case 1:
//         case 2:
//         case 3:
//         case 4:
//         case 5:
//         case 6:
//         case 7:
//     }

// }

int main() {
    struct addrinfo *infoaddr = nullptr; // Ponteiro para guardar informações do endereço
    char buffer[BUFFER_SIZE];
    //char msgbuffer[BUFFER_SIZE];
    string sendmsg;  
    int code;
    

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
        if((code= case_(sendmsg)) == -1)
            continue;
        // else if (code > 0)
        //     add_args(sendmsg, code);

        printf("%s", "a");
        // Envia mensagem
        const char* csendmsg = sendmsg.c_str(); 
        if (send_socket_udp_player(fd_udp, csendmsg , infoaddr) < 0)   {
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
