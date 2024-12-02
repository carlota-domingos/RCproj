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

int flag = 1;
class game_player {
public:
    std::string plid;
    int nT;

    game_player(const std::string& id) : plid(id), nT(0){}

    void reset() {
        nT = 0;
        plid = "none";
    }

    void next_try() {
        nT++;
    }

    bool same_try(int server_try) const {
        return nT == server_try;
    }
};

game_player curr_game = game_player("none12");

int add_args(string &msg, int code){
    if (code<= 2){
        return 0;
    }
    else if(curr_game.plid.compare("none12") == 0 and code != 5){
        printf("Erro: não existe um jogo ativo de momento\n");
        msg = "";
        return -1;
    } 
    if (code == 6) {
        size_t pos = msg.find("nT");
        if (pos != string::npos) {
            msg.replace(pos, 2, std::to_string(curr_game.nT));
        }
    }
    size_t pos = msg.find("PLID");
    if (pos != string::npos) {
        msg.replace(pos, 4, curr_game.plid);
    }
    return 0;
}

int case_server(const char* buffer_received) {
    std::string buffer(buffer_received); // Converte o buffer recebido em std::string
    std::string PLID; // Para armazenar o PLID

    std::cout << "Buffer recebido server: '" << buffer << "'" << std::endl;

    
}

int main() {
    struct addrinfo *infoaddr = nullptr; // Ponteiro para guardar informações do endereço
    char buffer[BUFFER_SIZE];
    string sendmsg;  
    int code;
    int n=0;

    // Inicializa o socket
    int fd_udp = init_socket_player("localhost", infoaddr);
    if (fd_udp < 0) {
        return 1;
    }
   
   //loop para durante o jogo                                                     
    while (flag == 1) {
        if (get_msg(sendmsg) != 0)  {
            perror("Erro ao ler a mensagem");
            return -1;
        } 

        if((code= case_terminal(sendmsg)) == -1 || add_args(sendmsg, code) == -1)
            continue;
        else if (code == 5)
            flag = 0;
        cout << "msg: " << sendmsg << endl;
        const char* csendmsg = sendmsg.c_str(); // Converte a string para um array de caracteres
        if (code== 0 || code == 3){
            //tcp
        } else {
            if (send_socket_udp_player(fd_udp, csendmsg , infoaddr) < 0)   {
                freeaddrinfo(infoaddr);
                close(fd_udp);
                return 1;
            }

            // Recebe mensagem
            n = receive_socket_udp_player(fd_udp, buffer, BUFFER_SIZE);
            if (n < 0)  {
                freeaddrinfo(infoaddr);
                close(fd_udp);
                return 1;
            }
            else{
               case_server(buffer);
            }
            

        }
        
        if (n !=0){
            write(1, "echo: ", 6);
            write(1, buffer, n);
            write(1, "\n", 1);
        }
    
        // Imprime a mensagem recebida
        memset(buffer, 0, n);
    }   
    // Limpeza
    freeaddrinfo(infoaddr);
    close(fd_udp);
    return 0;
}
