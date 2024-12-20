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
#include "libPlayer.h"
#include "udpTcp.h"
#include <iostream>
#include <string>

using namespace std;

#define BUFFER_SIZE 128
#define TCP_BUFFER_SIZE 1024
int flag = 1;
game_player curr_game= game_player("000000");

// FUnção que trata da mensagem que o server mandou
int case_server(const char *buffer_received, int code, string &sendmsg){
    string buffer(buffer_received); 
    string file_out;
    // Verefica o status do player em debug mode
    if (buffer.substr(0, 3) == "RDB") {
        if (buffer == "RDB OK\n") {
            cout << "Pode começar a jogar :)" << endl;
            curr_game.reset();
            curr_game.plid = sendmsg.substr(4, 6); 
        }
        else if (buffer == "RDB NOK\n") {
            cout << "Player already in a game" << endl;
        }
        else if (buffer == "RDB ERR\n") {
            cout << "Incorrect arguments given" << endl;
        }
    }
    // Verefica o status do player 
    else if (buffer.substr(0, 3) == "RSG") {
        if (buffer == "RSG OK\n") {
            std::cout << "Pode começar a jogar :)" << std::endl;
            curr_game.reset();
            curr_game.plid = sendmsg.substr(4, 6);
        }
        else if (buffer == "RSG NOK\n") {
            cout << "Player already in a game" << endl;
        }
        else if (buffer == "RSG ERR\n") {
            cout << "Incorrect arguments given" << endl;
        }
    }

    // Verefica o player tem um jogo ativo pelo ficheiro
    else if (buffer.substr(0, 3) == "RST") {
        if (buffer.substr(0, 7) == "RST ACT") {
            get_file_msg(buffer, file_out);
            cout << file_out << endl;
        }
        else if (buffer.substr(0, 7) == "RST FIN") {
            get_file_msg(buffer, file_out);
            cout << file_out << endl;
            curr_game.finish();
        }
        else if (buffer.substr(0, 7) == "RST NOK") {
            cout << "Não existe jogos ativos ou passados do player" << endl;
        }
    }
    // Verifica a scoreboard 
    else if (buffer.substr(0, 3) == "RSS") {
        
        if (buffer == "RSS EMPTY\n") {
            cout << "Não existem scores para a scoreboard" << endl;
        }
        else if (buffer.substr(0, 6) == "RSS OK") {
            get_file_msg(buffer, file_out);
            cout << file_out << endl;
        }
    }
    // Resposta ao quit 
    else if (buffer.substr(0, 3) == "RQT") {
        if (buffer.substr(0, 6) == "RQT OK") {
            cout << "Jogo terminado com sucesso" << endl;
            cout << "Codigo: " << buffer.substr(7, 7) << endl;
            curr_game.finish();
        }
        else if (buffer.substr(0, 7) == "RQT NOK" && flag == 1) {
            if (curr_game.active) {
                curr_game.finish();
                cout << "Tempo Esgotado. Jogo já foi terminado." << endl;
            }
            else
                cout << "Nao existe jogo ativo" << endl;
        }
        else if (buffer.substr(0, 7) == "RQT ERR")
            cout << "Erro ao terminar o jogo." << endl;
    }
    else if (buffer.substr(0, 3) == "RTR") {
        if (buffer.substr(0, 6) == "RTR OK") {
            cout << "Tentativa numero " << curr_game.nT << endl;
            cout << "\n";
            cout << "nB: " << buffer.substr(9, 1) << " nW: " << buffer.substr(11, 1) << endl;
            try {
                if (std::stoi(buffer.substr(7, 1)) == curr_game.nT)
                    curr_game.next_try();
            }
            catch (const std::exception &e){
                cout << "Erro ao converter o numero de tentativas" << endl;
            }
            if (buffer.substr(9, 1) == "4") {
                cout << "Jogo Ganho !" << endl;
                if (buffer.size() > 15){
                    cout << "Codigo: " << buffer.substr(13,7) << endl;
                    curr_game.finish();
                }
            }
        }
            else if (buffer == "RTR ERR\n")
                cout << "Argumentos inválidos" << endl;
            
            else if (buffer.substr(0, 7) == "RTR ETM") {
                cout << "Tempo esgotado. Codigo: " << buffer.substr(8, 7) << endl;
                curr_game.finish();
            }
            else if (buffer.substr(0, 7) == "RTR ENT") {
                cout << "Numero de tentativas Esgotado. Código: " << buffer.substr(8, 7) << endl;
                curr_game.finish();
                // dar throw de erro aqui
            }
            else if (buffer == "RTR NOK\n") {
                curr_game.finish();
                cout << "Tentativa fora de contexto." << endl;
            }
            else if (buffer == "RTR INV\n")
                cout << "Erro na comunicação " << endl;

            else if (buffer == "RTR DUP\n")
                cout << "Tentativa duplicada" << endl;
    }
    else if (buffer == "ERR\n")
        cout << "Erro ao enviar a mensagem" << endl;
    return 0;
}

// Função main do player
int main(int argc, char *argv[]){
    char *gs_ip = strdup("193.136.138.142");
    char *gs_port = strdup("58081");

    if (!gs_ip || !gs_port) {
        cerr << "Memory allocation failed" << endl;
        return 1;
    }

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-n") == 0 && i + 1 < argc) {
            free(gs_ip); // Free previous value
            gs_ip = strdup(argv[++i]);
            if (!gs_ip) {
                cerr << "Memory allocation failed" << endl;
                free(gs_port);
                return 1;
            }
        } else if (strcmp(argv[i], "-p") == 0 && i + 1 < argc) {
            free(gs_port); // Free previous value
            gs_port = strdup(argv[++i]);
            if (!gs_port) {
                cerr << "Memory allocation failed" << endl;
                free(gs_ip);
                return 1;
            }
        } else {
            cerr << "Usage: " << argv[0] << " [-n GSIP] [-p GSport]" << endl;
            free(gs_ip);
            free(gs_port);
            return 1;
        }
    }

    cout << "Servidor: " << gs_ip << endl;
    cout << "Porta: " << gs_port << endl;
    struct addrinfo *infoaddr = nullptr;
    struct addrinfo *infoaddr_tcp = nullptr;
    char buffer[BUFFER_SIZE];
    char buffer_tcp[TCP_BUFFER_SIZE];
    string sendmsg;
    int code;
    int n = 0;

    // Inicializa o socket
    int fd = init_socket_player(gs_ip, infoaddr, gs_port);
    if (fd < 0) {
        return 1;
    }

    while (flag == 1) {
        if (get_msg(sendmsg) != 0){
            perror("Erro ao ler a mensagem");
            return -1;
        }
        cout << "\n" ;

        if ((code = case_terminal(sendmsg)) == -1 || add_args(sendmsg, code, curr_game) == -1) {
            cout <<"\n";
            cout << "---------------------------------" << endl;
            cout <<"\n";
            continue;
        }
        else if (code == 5) {
            flag = 0;
        }
        else if ((code == 1 || code == 2) && check_active_game(sendmsg, curr_game) == 1){
            cout <<"\n";
            cout << "---------------------------------" << endl;
            cout <<"\n";
            continue;
        }

        sendmsg = sendmsg + '\n';
        const char *csendmsg = sendmsg.c_str(); 
        if (code == 0 || code == 3) { 
            int fd_tcp = init_tcp_player(gs_ip, infoaddr_tcp, gs_port);
            if (fd_tcp < 0) {
                freeaddrinfo(infoaddr_tcp);
                freeaddrinfo(infoaddr);
                close(fd);
                return 1;
            }
            // mensagem por tcp
            if ((n = send_tcp_player(fd_tcp, csendmsg)) < 0)  {
                printf("erro a enviar a mensagem");
                freeaddrinfo(infoaddr);
                freeaddrinfo(infoaddr_tcp);
                close(fd_tcp);
                close(fd);
                return 1;
            }
            // Recebe mensagem
            n = receive_tcp_player(fd_tcp, buffer_tcp, TCP_BUFFER_SIZE);
            if (n < 0) {
                printf("erro a receber a mensagem");
                freeaddrinfo(infoaddr);
                close(fd_tcp);
                close(fd);
                freeaddrinfo(infoaddr_tcp);
                return 1;
            }
            else if (n == 0) {
                cout << "No message received." << endl;
            }
            else {
                case_server(buffer_tcp, code, sendmsg);
            }
            close(fd_tcp);
            freeaddrinfo(infoaddr_tcp);
        }
        
        else { 
            // mensagem por udp
            if (send_socket_udp_player(fd, csendmsg, infoaddr) < 0) {
                freeaddrinfo(infoaddr);
                close(fd);
                return 1;
            }
            // Recebe mensagem
            int n = receive_socket_udp_player(fd, buffer, BUFFER_SIZE);
            if (n < 0) {
                freeaddrinfo(infoaddr);
                close(fd);
                return 1;
            }
            else if (n == 0) {
                cout << "No message received." << endl;
            }
            else {
                case_server(buffer, code, sendmsg);
            }
        }
        cout <<"\n";
        cout << "---------------------------------" << endl;
        cout <<"\n";
        sendmsg = "";
    }
    freeaddrinfo(infoaddr);
    close(fd);
    printf("A sair do jogo\n");
    return 0;
}
