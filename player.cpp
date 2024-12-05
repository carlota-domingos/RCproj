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


#define PORT "58011"
#define BUFFER_SIZE 128

int flag = 1;
class game_player {
public:
    std::string plid;
    int nT;

    game_player(const std::string& id) : plid(id), nT(0){}

    void reset() {
        nT = 1;
        plid = "none12";
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
#include <iostream>
#include <string>

using namespace std;

int case_server(const char* buffer_received, int code, string &sendmsg) {
    std::string buffer(buffer_received); // Convert the received buffer to std::string
    
    std::cout << "Buffer recebido server: '" << buffer << "'" << std::endl;

    if (buffer.substr(0, 3) == "RDB") {
        if (buffer == "RDB OK\n") {
            std::cout << "Jogo iniciado com sucesso" << std::endl;
            curr_game.reset();
            curr_game.plid = sendmsg.substr(4, 6); 
            printf("PLID: %s\n", curr_game.plid.c_str());
        } else if (buffer == "RDB NOK\n") {
            cout << "Player already in a game" << endl;
        } else if (buffer == "RDB ERR\n") {
            cout << "Incorrect arguments given" << endl;
        }
    } else if (buffer.substr(0, 3) == "RSG") {
        if (buffer == "RSG OK\n") {
            std::cout << "Jogo iniciado com sucesso" << std::endl;
            curr_game.reset();
            curr_game.plid = sendmsg.substr(4, 6); 
            printf("PLID: %s\n", curr_game.plid.c_str());
        } else if (buffer == "RSG NOK\n") {
            cout << "Player already in a game" << endl;
        } else if (buffer == "RSG ERR\n") {
            cout << "Incorrect arguments given" << endl;
        }
    } else if (buffer.substr(0, 3) == "STR") {
        if (buffer.substr(0, 6) == "STR OK") {
            cout << "Numero de tentativas: " << buffer.substr(8, 1) << endl;
        }
    } else if (buffer.substr(0, 3) == "RQT") {
        if (buffer.substr(0, 6) == "RQT OK") {
            cout << "Jogo terminado com sucesso" << endl;
            cout << "Codigo: " << buffer.substr(7, 7) << endl;
            curr_game.reset();
        } else if (buffer.substr(0, 7) == "RQT NOK") {
            cout << "Nao existe jogo ativo" << endl;
        } else if (buffer.substr(0, 7) == "RQT ERR") {
            cout << "Erro ao terminar o jogo." << endl;
        }
    } else if (buffer.substr(0, 3) == "RTR") {
        if (buffer.substr(0, 6) == "RTR OK") {
            curr_game.next_try();
            cout << "nB: " << buffer.substr(9, 1) << " nW: " << buffer.substr(11, 1) << endl;
            //checkar se o jogo acabou
        } else if (buffer == "RTR ERR\n") {
            cout << "Argumentos inválidos" << endl;
        } else if (buffer.substr(0, 7) == "RTR ETM") {
            cout << "Tempo esgotado. Codigo: " << buffer.substr(8, 7) << endl;
            curr_game.reset();
        } else if (buffer.substr(0, 7) == "RTR ENT") {
            cout << "Numero de tentativas Esgotado. Código: " << buffer.substr(8, 7) << endl;
            curr_game.reset();
        } else if (buffer== "RTR NOK\n") {
            cout << "Nao existe jogo ativo" << endl;
        } else if (buffer== "RTR INV\n") {
            cout << "Erro na comunicação" << endl;
        } else if (buffer == "RTR DUP\n") {
            cout << "Tentativa duplicada" << endl;
        }
    }

    return 0;
}

int main() {
    struct addrinfo *infoaddr = nullptr; // Ponteiro para guardar informações do endereço
    char buffer[BUFFER_SIZE];
    string sendmsg;  
    int code;
    int n=0;

    // Inicializa o socket
    int fd_udp = init_socket_player("193.136.138.142", infoaddr);
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
        sendmsg = sendmsg + '\n';
        std::cout << "msg: '" << sendmsg << "'" << std::endl;
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
               case_server(buffer, code, sendmsg);
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
